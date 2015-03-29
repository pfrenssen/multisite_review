<?php

/**
 * @file
 * Scans php code for debug statements.
 */

/**
 * Looks for debugstatements in the tested php code.
 */
class MultisiteReviewDebug extends MultisiteQaReview implements MultisiteQaReviewInterface {
  public static $extensionTypes = array('module', 'feature');
  public static $name = 'Exported fields';

  /**
   * Run the test.
   */
  public function runTest() {
    // Build a list of files to scan.
    $extension_path = dirname($this->extension->filename);
    $types = implode(',', array(
      'inc',
      'php',
      'module',
      'install',
    ));
    $glob_path = $extension_path . '/*.{' . $types . '}';
    $paths = glob($glob_path, GLOB_BRACE);

    // Scan each file for debug statements.
    foreach ($paths as $path) {
      $file = new SplFileObject($path);
      $statement_on_line = array();
      $line_number = 0;
      while (!$file->eof()) {
        $line = $file->current();
        $statements = array(
          'debug',
          'dpm',
          'dsm',
          'dpq',
          'kpr',
          'print_r',
          'var_dump',
          'dps',
        );
        foreach ($statements as $statement) {
          if (preg_match('/' . preg_quote($statement . '(') . '/', $line) === 1) {
            $statement_on_line[] = $line_number;
          }
        }
        $file->next();
        $line_number++;
      }

      if (!empty($statement_on_line)) {
        $this->setMessage(dt('@file contains a debug statement on line(s) @line', array(
          '@file' => basename($path),
          '@line' => implode(',', $statement_on_line),
        )), 'warning');
      }
    }
  }
}
