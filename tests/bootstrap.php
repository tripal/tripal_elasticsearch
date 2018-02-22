<?php
test_suite_read_and_set_environment();

$drupal_root = getenv('DRUPAL_ROOT');
define('DRUPAL_ROOT', $drupal_root ? $drupal_root : __DIR__.'/../../../../..');
require_once DRUPAL_ROOT.'/includes/bootstrap.inc';
$_SERVER['REMOTE_ADDR'] = '127.0.0.1';

$current_dir = getcwd();
chdir(DRUPAL_ROOT);

// Bootstrap Drupal.
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

require_once __DIR__.'/../tripal_elasticsearch.module';

chdir($current_dir);

/**
 * @throws \Exception
 */
function test_suite_read_and_set_environment() {
    $filename = __DIR__.'/../.env';
    if(file_exists($filename)) {
        $file = fopen($filename, 'r');
        while ($line = readline($file)) {
            // break line into key value
            $env = explode('=', $line);
            if(count($env) === 2) {
                putenv($line);
            } else {
                throw new Exception('Invalid environment line: ' . $line);
            }
        }
        fclose($file);
    }
}
