<?php

/**
 * Basic QA for Multisite
 *
 * @return array
 */
function multisite_review_qa_basic () {
  $result = TRUE;
  $modules = module_list();
  $findings = array();
  foreach($modules as $machine_name => $module) {
    $path = drupal_get_path('module', $machine_name) . '/' . $machine_name . '.info';
    $is_custom = preg_match('/\/custom\//', $path);
    $is_feature = preg_match('/\/features\//', $path);
    $type = $is_feature ? 'feature' : 'module';
    $standards = array();
    $standards['php_version'] = '5.2.4';
    $standards['required_info_keys'] = array('name', 'description', 'version', 'package', 'core');
    $standards['required_info_keys_module'] = array();
    $standards['required_info_keys_feature'] = array('php', 'features');
    $standards['features_api_version'] = 'api:1';
    // Check customs only
    if(!$is_custom) {
      continue;
    }

    $info = drupal_parse_info_file($path);
    $info_keys_to_check = array_merge($standards['required_info_keys'], $standards['required_info_keys_' . $type]);
    foreach($info_keys_to_check as $key) {
      if(!array_key_exists($key, $info)) {
        $findings[$type][$machine_name][$key] = t('The %type\'s %key is not set in %module.info file.', array('%module' => $machine_name, '%key' => $key, '%type' => $type));
      }
    }
    if(isset($info['php']) && $info['php'] !== $standards['php_version']) {
      $findings[$type][$machine_name]['php_version'] = t('Incorrect PHP version. Required: %php_version', array('%php_version' => $standards['php_version']));
    }
    // Features only
    if($is_feature) {
      if(isset($info['features']['features_api'][0]) && $info['features']['features_api'][0] !== $standards['features_api_version']) {
        $findings[$type][$machine_name]['features_api_version'] = t('Incorrect Features API version. Required: %features_api_version', array('%features_api_version' => $standards['features_api_version']));
      }
      elseif (!isset($info['features']['features_api'][0])) {
        $findings[$type][$machine_name]['features_api_version'] = t('Features API version not set in  %module.info file.', array('%module' => $machine_name));
      }
    }
  }

  return array('result' => empty($findings), 'value' => $findings);
}

function multisite_review_qa_basic_help($check = NULL, $skipped_message = NULL) {
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
          'html' => theme_item_list(array('items' => $findings, 'title' => $title, 'type' => 'ul', 'attributes' => array())),
        );
      }
    }
  }
  kpr($element);
  return $element;
}
