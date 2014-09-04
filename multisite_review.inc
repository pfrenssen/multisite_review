<?php

/**
 * @file
 * Automated QA of Multisite projects, built on the Security Review module.
 */

/**
 * Security Review callback. Performs QA for Multisite projects.
 *
 * @see hook_security_checks()
 * @see multisite_review_security_checks()
 */
function multisite_review_qa() {
  module_load_include('inc', 'multisite_review', 'multisite_review_module');

  $results = array();

  foreach (module_list() as $module_name) {
    $module = new MultisiteReviewModule($module_name);

    // Only check custom modules.
    if (!$module->isCustom()) {
      continue;
    }

    $results += multisite_review_check_info_file($module);
  }

  return array('result' => empty($results), 'value' => $results);
}

/**
 * Help callback for Security Review. Provides help for the QA check.
 */
function multisite_review_qa_help($check = NULL, $skipped_message = NULL) {
  module_load_include('inc', 'multisite_review', 'multisite_review_module');

  $element = array();
  $element['title'] = t('Basic Multisite QA checks');
  $element['descriptions'][] = t('The modules and features on the Multisite platform should adhere to the standards outlined in the !link.', array('!link' => l(t('Guidelines for feature creation'), 'https://webgate.ec.europa.eu/fpfis/wikis/display/MULTISITE/Build+custom+features')));

  if (!empty($skipped_message)) {
    $element['results']['descriptions'][] = $skipped_message;
  }
  elseif ($check && $check['result'] == FALSE) {
    $results_message = t('The following problems were identified:');
    $element['results']['descriptions'][] = $results_message;
    $element['findings']['descriptions'][] = $results_message;
    $values = $check['value'];
    foreach ($values as $module_name => $results) {
      $module = new MultisiteReviewModule($module_name);
      $title = $module_name . ' ' . $module->type;
      $element['findings']['items'][] = array(
        'html' => theme_item_list(array(
          'items' => $results,
          'title' => $title,
          'type' => 'ul',
          'attributes' => array(),
        )),
      );
    }
  }

  return $element;
}

/**
 * Performs automated review of the given module's .info file.
 *
 * @param MultisiteReviewModule $module
 *   The machine name of the module to review.
 *
 * @return array
 *   An array of failures.
 */
function multisite_review_check_info_file(MultisiteReviewModule $module) {
  module_load_include('inc', 'multisite_review', 'multisite_review_module');

  // Define a list of required keys. These are all specified in the custom
  // feature development guidelines.
  // @see https://webgate.ec.europa.eu/fpfis/wikis/display/MULTISITE/Build+custom+features
  $required_keys = array(
    'core',
    'description',
    'name',
    'package',
    'version',
  );

  // Features also require a 'features' key which lists the components that are
  // exported in the feature.
  if ($module->isFeature()) {
    $required_keys[] = 'features';
  }

  $results = array();

  $info = $module->getInfo();
  $name = $module->getName();
  foreach ($required_keys as $key) {
    if (!array_key_exists($key, $info)) {
      $args = array(
        '%module' => $name,
        '%key' => $key,
        '%type' => $module->type,
      );
      $results[$name][$key] = t("The %type's %key is not set in %module.info file.", $args);
    }
  }

  // If the module is a feature, check for some specific key/value pairs that
  // are required according to the guidelines:
  // - the minimum PHP version.
  // - the Features API version.
  if ($module->isFeature()) {
    $required_values = array();
    $required_values['php'] = MULTISITE_REVIEW_PHP_VERSION;
    $required_values['features']['features_api'][0] = MULTISITE_REVIEW_FEATURES_API_VERSION;

    if ($missing_values = drupal_array_diff_assoc_recursive($required_values, $info)) {
      $missing_keys = implode(', ', array_keys($missing_values));
      $results[$name]['missing_values'] = t('Missing required key/value pairs in the .info file: %keys', array('%keys' => $missing_keys));
    }
  }

  return $results;
}
