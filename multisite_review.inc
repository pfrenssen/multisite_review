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

  // Loop over the modules and perform the reviews.
  // @todo Allow the user to choose the modules and reviews.
  foreach (module_list() as $module_name) {
    $module = new MultisiteReviewModule($module_name);

    // Only check custom modules.
    if (!$module->isCustom()) {
      continue;
    }

    foreach (module_invoke_all('multisite_review_reviews') as $review => $info) {
      // Include the file if it has been specified.
      if (!empty($info['file'])) {
        if (is_file($file = DRUPAL_ROOT . '/' . $info['file'])) {
          require_once $file;
        }
      }

      // Perform the review.
      $result = $info['callback']($module);
      if (!empty($result)) {
        $results[$module_name][$review] = $result;
      }
    }
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

    // Extract and format the review results.
    // @todo Format this as a nice table.
    $values = $check['value'];
    foreach ($values as $module_name => $reviews) {
      $module = new MultisiteReviewModule($module_name);
      foreach ($reviews as $review => $results) {
        $title = t('@module @type @review', array(
          '@module' => $module_name,
          '@type' => $module->type,
          '@review' => $review,
        ));
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
  }

  return $element;
}
