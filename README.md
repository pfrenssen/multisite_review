Multisite review
================

Performs automated review of modules and features for the Multisite platform of the European Commission.

The Multisite Platform (aka FPFIS) is a web platform built on Drupal 7 that hosts websites for the EC.


## Dependencies

- None


## Usage

- Put this module in your ~/.drush folder to enable it globally.
- Run 'drush code-review MODULENAME' to review.


## Security Review

The original version of this tool was using the [Security Review](https://www.drupal.org/project/security_review)
module to run its tests. This is still available in the 7.x-1.x branch.
