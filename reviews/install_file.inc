<?php

/**
 * @file
 * Reviews for the module's .install file.
 */

/**
 * Performs automated review of the given module's .install file.
 *
 * @param MultisiteReviewModule $module
 *   The machine name of the module to review.
 *
 * @return array
 *   An array of failures.
 */
class MultisiteReviewInstallFile extends MultisiteQaReview implements MultisiteQaReviewInterface {
  public static $extensionTypes = array('feature');
  public static $name = 'Info file';

  /**
   * Run the test.
   */
  public function runTest() {
    // Check if the module has an install file. Skip the rest of the review when
    // missing.
    $filename = $this->extension->name . '.install';
    $install_path = dirname($this->extension->filename) . '/' . $filename;

    if (!file_exists($install_path)) {
      $this->setMessage(dt('No install file present.'), 'error');
      return;
    }

    // Load the install file.
    require_once $install_path;

    // Check if the enable and disable hooks output status messages.
    foreach (array('enable', 'disable') as $hook) {
      // If this function does not exist, the error is already logged.
      if (!function_exists($this->extension->name . '_' . $hook)) {
        continue;
      }
      $reflection = new ReflectionFunction($this->extension->name . '_' . $hook);
      $function_scope = '';
      $file = new SplFileObject($install_path);
      $file->seek($reflection->getStartLine() - 1);
      for ($i = $reflection->getStartLine(); !$file->eof() && $i < $reflection->getEndLine(); $i++) {
        $function_scope .= $file->current();
        $file->next();
      }
      // Look for drupal_set_message within the function scope.
      if (strpos($function_scope, 'drupal_set_message') === FALSE) {
        $this->setMessage(dt('@hook does not set the correct message.', array(
          '@hook' => "hook_$hook()",
        )), 'error');
      }
    }
  }
}
