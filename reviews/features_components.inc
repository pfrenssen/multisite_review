<?php

/**
 * @file
 * Reviews for the module's Features components.
 */

/**
 * Performs automated review of the given module's Features components.
 */
class MultisiteReviewFeaturesComponents extends MultisiteQaReview implements MultisiteQaReviewInterface {
  public static $extensionTypes = array('feature');
  public static $name = 'Features components';

  public $components_blacklist = array(
    'menu_custom',
   // 'menu_links',
  );

  /**
   * Run the test.
   */
  public function runTest() {
    // A list of Features components that should not be exported.

    $info = $this->extension->info;
    if (!empty($info['features'])) {
      if ($components = array_intersect($this->components_blacklist, array_keys($info['features']))) {
        foreach ($components as $component) {
          $this->setMessage(t('The module exports the blacklisted component !component.', array(
            '!component' => $component,
          )), 'error');
        }
      }
    }
  }
}
