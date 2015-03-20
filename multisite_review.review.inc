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

  /**
   * Returns help messages related to the generated errors.
   */
  public function help($component = 'general');

  /**
   * Store a message for in the report.
   */
  public function setMessage($message, $log_level = 'notice', $component = 'general');

  /**
   * Returns all set messages.
   */
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
  /**
   * Construct: Set the extension under test as a property.
   */
  public function __construct($extension) {
    $this->extension = $extension;
    $this->setMessage(dt("Checking @extension with '@name'.", array(
      '@extension' => $this->extension->name,
      '@name' => $this::$name,
    )), 'info');
  }

  /**
   * Build debendencies.
   *
   * Returns a list of all the extentensions that should be enabled for this
   * test to be able to run.
   */
  public function getDependencies() {
    return array();
    // If you need the module to be enabled:
    // return array($this->extension->name);
  }

  /**
   * Save the messages.
   */
  public function setMessage($message, $log_level = 'notice', $component = 'general') {
    $this->messages[] = array(
      'message' => $message,
      'log_level' => $log_level,
      'component' => $component,
    );
  }
  /**
   * Return all messages.
   */
  public function getMessages() {
    return $this->messages;
  }

  /**
   * Return help for set messages.
   *
   * The actual implementation is left to subclasses.
   */
  public function help($component = 'general') {
    return '';
  }
}
