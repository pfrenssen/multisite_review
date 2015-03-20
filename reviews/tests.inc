<?php

/**
 * @file
 * Checks if tests are present.
 */

/**
 * Performs automated review of test presence.
 */
class MultisiteReviewTests extends MultisiteQaReview implements MultisiteQaReviewInterface {
  public static $extensionTypes = array('feature', 'module');
  public static $name = 'Tests';

  public function runTest() {
    $path = dirname($this->extension->filename);
    // Check if the module has a 'tests' folder.
    if (!file_exists($path . '/tests')) {
      $this->setMessage(dt('The "/tests" folder is missing.'), 'warning');
      return;
    }

    // Check if the tests folder contains at least one test.
    if (!glob($path . '/tests/*.test')) {
      $this->setMessage(dt('There are no files matching the pattern "*.test" in the "/tests" folder.'), 'warning');
    }
  }
}
