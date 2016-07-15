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

  /**
   * Run the test.
   */
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

    // Check the info file for tests.
    if ($this->extension->extension_type != 'theme') {
      $this->checkTests();
    }
  }

  /**
   * Check if the module has at least one test.
   */
  public function checkTests() {
    $info = $this->extension->info;

    $has_tests = FALSE;
    if (array_key_exists('files', $info)) {
      foreach ($info['files'] as $file) {
        if (substr($file, -5) === '.test') {
          $has_tests = TRUE;
          break;
        }
      }
    }
    if (!$has_tests) {
      $this->setMessage(dt('There are no tests added to the files[] array.'), 'warning');
    }
  }

}
