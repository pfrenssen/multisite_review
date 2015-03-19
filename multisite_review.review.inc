<?php

/**
 * @file
 * The general structure all QA checks need to implement.
 */

/**
 * Interface describing the Qa check plugin structure.
 *
 * Please note that implementing class names should be derived from their
 * file path: my_awesome_check.inc => MultisiteReviewMyAwesomeCheck
 */
interface MultisiteQaReviewInterface {
  /**
   * Does the actual test-run on a module.
   */
  public function runTest();

  public function help($component = 'general');

  public function setMessage($message, $log_level = 'notice', $component = 'general');

  public function getMessages();
}

abstract class MultisiteQaReview {
  public $extension = NULL;
  protected $messages = array();

  /**
   * user facing name for the QA check.
   */
  public static $name = NULL;
  /**
   * The extension type this check supports.
   * e.g. module, feature or theme.
   */
  public static $extensionTypes = NULL;
  public function __construct($extension) {
    $this->extension = $extension;
    $this->setMessage(dt("Checking @extension with '@name'.", array(
      '@extension' => $this->extension->name,
      '@name' => $this::$name,
    )), 'info');
  }

  public function getDependencies() {
    return array();
    // If you need the module to be enabled:
    // return array($this->extension->name);
  }
  public function setMessage($message, $log_level = 'notice', $component = 'general') {
    $this->messages[] = array(
      'message' => $message,
      'log_level' => $log_level,
      'component' => $component,
    );
  }
  public function getMessages() {
    return $this->messages;
  }

  function help($component = 'general') {
    switch ($component) {
      default:
        $description = '';
        break;
    }
    return $description;
  }
}

