<?php

/**
 * @file
 * Reviews for the module's icon.
 */

/**
 * Performs automated review of the given module's icon.
 */
class MultisiteReviewIcon extends MultisiteQaReview implements MultisiteQaReviewInterface {
  public static $extensionTypes = array('feature');
  public static $name = 'Check icon';
  function runTest() {
    $filename = $this->extension->name . '_icon.png';
    $path = dirname($this->extension->filename) . '/' . $filename;
    if (!file_exists($path)) {
      $this->setMessage(dt('An icon with filename !filename is not present.', array(
        '!filename' => $filename,
      )), 'warning');
    }
  }
}

