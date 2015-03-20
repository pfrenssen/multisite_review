<?php

/**
 * @file
 * Reviews for the module's exported fields.
 */

/**
 * 
 */
class MultisiteReviewExportedFields extends MultisiteQaReview implements MultisiteQaReviewInterface {
  public static $extensionTypes = array('feature');
  public static $name = 'Exported fields';

  /**
   * Performs automated review of the given module's exported fields.
   */
  function runTest() {
    // Check if the feature exports fields. Skip the review if it doesn't.
    $info = $this->extension->info;
    if (empty($info['features']['field_base'])) {
      $this->setMessage('No field instances', 'info');
      return;
    }

    // Load the features definition of field bases.
    $feature_dir = dirname($this->extension->filename);
    $file_path = $feature_dir . '/' . $this->extension->name . '.features.field_base.inc';
    include_once $file_path;
    $field_base_hook = $this->extension->name . '_field_default_field_bases';
    $field_base_definition = $field_base_hook();

    // Can we get the field base definitions from the feature?
    if (!function_exists($field_base_hook)) {
      $this->setMessage('Unable to load field base from feature.', 'error');
      return;
    }

    // Iterate over all field bases.
    foreach ($info['features']['field_base'] as $field_name) {
      $field_export = $field_base_definition[$field_name];
      if (empty($field_export)) {
        $this->setMessage(t('Field "@id" is present in info file, but not in export.', array('@id' => $field_name)), 'error', 'lock');
        return;
      }
      // Check if the field is locked.
      if (empty($field_export['field_config']['locked'])) {
        $this->setMessage(t('Field "@field" is not locked.', array('@field' => $field_name)), 'warning', 'lock');
      }
      else {
        $this->setMessage(t('Field "@field" is locked.', array('@field' => $field_name)), 'info', 'lock');
      }
      // If it is a date field, check that it is configured to use UNIX time.
      if ($field_export['field_config']['module'] == 'date' && $field_export['field_config']['type'] != 'datestamp') {
        $this->setMessage(t('The @field field does not use UNIX time.', array('@field' => $field_name)), 'warning', 'datetime');
      }
    }
  }

  function help($component = 'general') {
    $description = NULL;
    switch ($component) {
      case 'lock':
        $description = dt('== All fields must be locked ==') . PHP_EOL;
        $description .= dt('To avoid inconsistency between the field settings on exported in features and the configuration in the database, it is required to lock all fields in the playground/production environment.') . PHP_EOL;
        $description .= dt('Read more about locking fields on the wiki: !link', array(
          '!link' => 'http://todo')
        ) . PHP_EOL;

        break;
      case 'general':
      default:
        break;
    }
    return $description;
  }

}
