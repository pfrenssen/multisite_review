<?php

/**
 * @file
 * Automated QA of Multisite projects, built on the Security Review module.
 */

/**
 * Security Review callback. Performs QA for Multisite projects.
 *
 * @see hook_security_checks()
 */
function multisite_review_qa() {
  $findings = array();

  foreach (module_list() as $module) {
    // Determine if the module is a custom module and/or a feature. This is
    // derived from the path where the module is installed.
    $path = drupal_get_path('module', $module) . '/' . $module . '.info';
    $is_custom = preg_match('/\/custom\//', $path);
    $is_feature = preg_match('/\/features\//', $path);
    $type = $is_feature ? 'feature' : 'module';

    $standards = array();
    $standards['php_version'] = '5.2.4';
    $standards['required_info_keys'] = array(
      'core',
      'description',
      'name',
      'package',
      'version',
    );
    $standards['required_info_keys_module'] = array();
    $standards['required_info_keys_feature'] = array('php', 'features');
    $standards['features_api_version'] = 'api:1';

    // Only check custom modules.
    if (!$is_custom) {
      continue;
    }

    $info = drupal_parse_info_file($path);
    $info_keys_to_check = array_merge($standards['required_info_keys'], $standards['required_info_keys_' . $type]);
    foreach ($info_keys_to_check as $key) {
      if (!array_key_exists($key, $info)) {
        $args = array(
          '%module' => $module,
          '%key' => $key,
          '%type' => $type,
        );
        $findings[$type][$module][$key] = t("The %type's %key is not set in %module.info file.", $args);
      }
    }
    if (isset($info['php']) && $info['php'] !== $standards['php_version']) {
      $findings[$type][$module]['php_version'] = t('Incorrect PHP version. Required: %php_version', array('%php_version' => $standards['php_version']));
    }

    // Perform some additional checks if the module is a Feature.
    if ($is_feature) {
      if (isset($info['features']['features_api'][0]) && $info['features']['features_api'][0] !== $standards['features_api_version']) {
        $findings[$type][$module]['features_api_version'] = t('Incorrect Features API version. Required: %features_api_version', array('%features_api_version' => $standards['features_api_version']));
      }
      elseif (!isset($info['features']['features_api'][0])) {
        $findings[$type][$module]['features_api_version'] = t('Features API version not set in %module.info file.', array('%module' => $module));
      }
    }
  }

  return array('result' => empty($findings), 'value' => $findings);
}

/**
 * Help callback for Security Review. Provides help for the QA check.
 */
function multisite_review_qa_help($check = NULL, $skipped_message = NULL) {
  $element = array();
  $element['title'] = t('Basic Multisite QA checks');
  $element['descriptions'][] = t('Multisite standards can be found here');

  if (!empty($skipped_message)) {
    $element['findings']['descriptions'][] = $skipped_message;
  }
  elseif ($check && $check['result'] == FALSE) {
    $element['findings']['descriptions'][] = t('The following problems were identified:');
    $values = $check['value'];
    foreach ($values as $type => $type_findings) {
      foreach ($type_findings as $module => $findings) {
        $title = $module . ' ' . $type;
        $element['findings']['items'][] = array(
          'html' => theme_item_list(array(
            'items' => $findings,
            'title' => $title,
            'type' => 'ul',
            'attributes' => array(),
          )),
        );
      }
    }
  }

  return $element;
}
