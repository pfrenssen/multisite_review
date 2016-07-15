<?php
/**
 * @file
 * Builds a code review report.
 */

/**
 * The PHP version that should be present in reviewed modules' .info files.
 */
const MULTISITE_REVIEW_PHP_VERSION = '5.2.4';

/**
 * The Features API version that is currently supported.
 */
const MULTISITE_REVIEW_FEATURES_API_VERSION = 'api:2';

/**
 * The current multisite platform version.
 */
const MULTISITE_REVIEW_MULTISITE_VERSION = '1.7';

require_once __DIR__ . '/multisite_review.review.inc';

/**
 * Gets the php version to test for.
 *
 * @return string
 *   The php version.
 */
function _get_php_version() {
  if (function_exists('drush_get_option')) {
    return drush_get_option('php-version', MULTISITE_REVIEW_PHP_VERSION);
  }
  return MULTISITE_REVIEW_PHP_VERSION;
}

/**
 * Gets the multisite version to test for.
 *
 * @return string
 *   The multisite version.
 */
function _get_multisite_version() {
  if (function_exists('drush_get_option')) {
    return drush_get_option('multisite-version', MULTISITE_REVIEW_MULTISITE_VERSION);
  }
  return MULTISITE_REVIEW_MULTISITE_VERSION;
}

/**
 * Implements hook_drush_command().
 */
function multisite_review_drush_command() {
  $items = array();
  $items['code-review'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_MAX,
    'description' => 'Review an extension (module, feature or theme) for multi-site usage.',
    'arguments' => array(
      'extensions' => 'A list of modules or themes to validate. You can use the * wildcard at the end of extension names to review all matches.',
    ),
    'options' => array(
      'show-success' => 'Show successful test results.',
      // @todo
      'dynamic-analysis' => 'Activate the dynamic analysis checks. (defaults to static analysis only)',
      // @todo
      'checks' => 'Provide an explicit list of checks to run. Defaults to all.',
      'exclude-checks' => 'Comma separated list of checks to exclude',
      'php-version' => 'The php version to check for.',
      'multisite-version' => 'The multisite version to check for.',
      'break-cphp' => 'To break or not on cphp.',
      'check-directory' => 'The directory to check for (example: sites/all)',
    ),
    'aliases' => array('cr'),
  );
  return $items;

}

/**
 * Drush command validation callback: code review.
 */
function drush_multisite_review_code_review_validate() {
  $args = pm_parse_arguments(func_get_args());

  $extension_info = drush_get_extensions();

  // Filter modules not in sites/all.
  foreach ($extension_info as $key => $module) {
    if (drush_get_option('check-directory') && strpos($module->filename, drush_get_option('show-success')) === FALSE) {
      unset($extension_info[$key]);
    }
  }

  $extensions = array();
  // Classify $args in themes, modules, features or unknown.
  drush_multisite_review_classify_extensions($args, $extensions, $extension_info);
  $unknown = array_diff($args, $extensions);

  // Bail out is the user specified an non-existing extension.
  if (!empty($unknown)) {
    foreach ($unknown as $key => $name) {
      drush_log(dt('!extension was not found.', array('!extension' => $name)), 'warning');
    }
  }
  // Bail out if no extension provided on the commandline.
  if (empty($extensions)) {
    drush_log(dt('Please supply a valid extension to test.'), 'warning');
    return FALSE;
  }
  drush_set_context('MULTISITE_REVIEW_EXTENSION_INFO', $extension_info);
  drush_set_context('MULTISITE_REVIEW_EXTENSIONS', $extensions);
  return TRUE;
}

/**
 * Classify extensions as modules, themes or features.
 */
function drush_multisite_review_classify_extensions(&$extensions, &$extension_list, $extension_info) {
  _drush_pm_expand_extensions($extensions, $extension_info);
  foreach ($extensions as $extension) {
    if (!isset($extension_info[$extension])) {
      continue;
    }
    $extension_list[$extension] = $extension;
    if ($extension_info[$extension]->type == 'module') {
      if (!empty($extension_info[$extension]->info['features'])) {
        $extension_info[$extension]->extension_type = 'feature';
      }
      else {
        $extension_info[$extension]->extension_type = 'module';
      }
    }
    else {
      if ($extension_info[$extension]->type == 'theme') {
        $extension_info[$extension]->extension_type = 'theme';
      }
    }
  }
}

/**
 * Drush command callback: Code review.
 *
 * @todo Clean this up, complexity is too high.
 */
function drush_multisite_review_code_review() {
  module_load_include('inc', 'multisite_review');

  $dependencies = array();
  $tests_to_run = multisite_review_build_list_of_tests($dependencies);
  multisite_review_enable_dependencies($dependencies);

  // Run the tests.
  foreach ($tests_to_run as $test) {
    $test->runTest();
  }

  // Aggregate results.
  $verbose = drush_get_context('DRUSH_VERBOSE');
  // Show notices, etc. by setting verbose mode for drush_log().
  drush_set_context('DRUSH_VERBOSE', TRUE);
  $printed_descriptions = array();
  $previous_name = '';
  foreach ($tests_to_run as $test) {
    $messages = $test->getMessages();
    foreach ($messages as $message) {
      $show_success = drush_get_option('show-success');
      $log_levels = array('notice', 'message', 'info');
      if (!in_array($message['log_level'], $log_levels) || $show_success) {
        // If needed, print some information about the error.
        if (empty($printed_descriptions[$test::$name][$message['component']])) {
          $printed_descriptions[$test::$name][$message['component']] = TRUE;
          $help = $test->help($message['component']);
          if ($help) {
            drush_print($help);
          }
        }
        $name = $test->extension->name;
        if ($name <> $previous_name) {
          drush_print(PHP_EOL . $name . ':');
        }
        drush_log($name . " --> " . $message['message'], $message['log_level']);

        $previous_name = $name;
      }
    }
  }

  // If there was any error. Throw an exception to make cphp break.
  if (isset($previous_name) && drush_get_option('break-cphp')) {
    drush_set_error('', 'There are some checks that did not pass tests.');
  }

  // Restore verbosity level.
  drush_set_context('DRUSH_VERBOSE', $verbose);

  return TRUE;
}

/**
 * Build a list of tests to run.
 */
function multisite_review_build_list_of_tests(&$dependencies) {
  $extensions_info = drush_get_context('MULTISITE_REVIEW_EXTENSION_INFO');
  $extensions_list = drush_get_context('MULTISITE_REVIEW_EXTENSIONS');
  $check_list = multisite_review_get_checks();

  // Get the list of checks.
  if ($exclude = drush_get_option('exclude-checks')) {
    $check_list = array_flip(array_diff(array_flip($check_list), explode(',', $exclude)));
  }

  foreach ($extensions_list as $extension_name) {
    foreach ($check_list as $machine_name => $class_name) {
      $extension_info = $extensions_info[$extension_name];
      // Skip this test if the type is not supported.
      if (!in_array($extension_info->extension_type, $class_name::$extensionTypes)) {
        drush_log(dt("Cannot check @extension (@type) with @machine (supports @compatibility), as these are incompatible.", array(
          '@extension' => $extension_name,
          '@type' => $extension_info->extension_type,
          '@machine' => $machine_name,
          '@compatibility' => implode(', ', $class_name::$extensionTypes),
        )));
        continue;
      }
      $extension_info = $extensions_info[$extension_name];

      $test = new $class_name($extension_info);
      $tests_to_run[] = $test;
      $dependencies += $test->getDependencies();
    }
  }
  return $tests_to_run;
}

/**
 * Enables the required dependencies.
 */
function multisite_review_enable_dependencies($dependencies) {
  $extension_info = drush_get_context('MULTISITE_REVIEW_EXTENSION_INFO');
  $dependencies_disabled = array();
  if (!empty($dependencies)) {
    $dependencies = array_unique($dependencies);
    $dependencies_disabled = array();
    foreach ($dependencies as $name) {
      if (!$extension_info[$name]->status) {
        $dependencies_disabled[] = $name;
      }
    }

  }
  if (!empty($dependencies_disabled)) {
    $drush_affirmative = drush_get_context('DRUSH_AFFIRMATIVE');
    drush_set_context('DRUSH_AFFIRMATIVE', FALSE);
    // Invoke a new process to enable dependencies.
    drush_invoke_process('@self', 'pm-enable', $dependencies_disabled, array(), array('interactive' => TRUE));
    // Restore DRUSH_AFFIRMATIVE context.
    drush_set_context('DRUSH_AFFIRMATIVE', $drush_affirmative);
  }
}

/**
 * Get all the possible review plugins that can be run against an extension.
 */
function multisite_review_get_checks() {
  // Iterate over all inc files in the plugin directory.
  $check_base_path = __DIR__ . '/reviews/*.inc';
  $check_paths = glob($check_base_path);
  $checks = NULL;
  foreach ($check_paths as $check_path) {
    require_once $check_path;
    $check_filename = basename($check_path, '.inc');
    $class_name = multisite_review_filename_to_classname($check_filename);
    if (class_exists($class_name)) {
      // Add all known good plugins.
      $checks[$check_filename] = $class_name;
    }
  }
  return $checks;
}

/**
 * Convert a filename to a classname.
 */
function multisite_review_filename_to_classname($filename) {
  // Convert the filename to a class name.
  $check_filename_parts = explode('_', $filename);
  $check_filename_parts_uc = array_map('ucfirst', $check_filename_parts);
  $class_name = 'MultisiteReview' . implode('', $check_filename_parts_uc);
  return $class_name;
}
