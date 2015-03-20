<?php

/**
 * @file
 * Reviews for the module's exported user permissions.
 */

/**
 * Performs automated review of the given module's exported user permissions.
 *
 * @param MultisiteReviewModule $module
 *   The machine name of the module to review.
 *
 * @return array
 *   An array of failures.
 */
class MultisiteReviewUserPermissions extends MultisiteQaReview implements MultisiteQaReviewInterface {
  public static $extensionTypes = array('feature');
  public static $name = 'User permissions';

  public function runTest() {
    // A list of permissions that should not be exported.
    $permission_blacklist = array(
      'administer features',
      'administer modules',
      'administer software updates',
      'manage features',
    );

    // A list of modules that should not have any of their permissions exported.
    $module_blacklist = array(
      'devel',
      'php',
    );

    // Check if the feature exports user permissions. Skip the review if it
    // doesn't.
    $info = $this->extension->info;
    if (empty($info['features']['user_permission'])) {
      return array();
    }
    $permission_mapping = user_permission_get_modules();
    foreach ($info['features']['user_permission'] as $permission) {
      // Check if any of the blacklisted permissions are exported.
      if (in_array($permission, $permission_blacklist)) {
        $this->setMessage(dt('The blacklisted "@permission" permission is exported.', array(
          '@permission' => $permission,
        )), 'warning');
      }
      // Check if the permission is related to a blacklisted module.
      if (isset($permission_mapping[$permission])) {
        $module = $permission_mapping[$permission];
        if (in_array($module, $module_blacklist)) {
          $this->setMessage(dt('The "@permission" permission from the blacklisted module "@module" is exported.', array(
            '@permission' => $permission,
            '@module' => $module,
          )), 'warning');
        }
      }
      // Unable to determine the module providing this permission.
      else {
        $this->setMessage(t('Unable to determine the module providing "@permission". Maybe it\'s not enabled?', array(
          '@permission' => $permission,
        )), 'warning');
      }
    }
  }
}