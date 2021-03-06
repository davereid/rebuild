<?php

/**
 * @file
 * Functions for rebuilding a local development environment.
 */

if (file_exists(dirname(dirname(DRUSH_BASE_PATH)) . '/autoload.php')) {
  require_once dirname(dirname(DRUSH_BASE_PATH)) . '/autoload.php';
}

// Main classes.
require_once 'src/DrushRebuild/DrushRebuild.php';
require_once 'src/DrushRebuild/Diagnostics.php';

// Components.
require_once 'src/DrushRebuild/Rebuilder/Component/SiteInstall.php';
require_once 'src/DrushRebuild/Rebuilder/Component/SqlSync.php';
require_once 'src/DrushRebuild/Rebuilder/Component/PanSqlSync.php';
require_once 'src/DrushRebuild/Rebuilder/Component/Rsync.php';
require_once 'src/DrushRebuild/Rebuilder/Component/DrushScript.php';
require_once 'src/DrushRebuild/Rebuilder/Component/Module.php';
require_once 'src/DrushRebuild/Rebuilder/Component/UserLogin.php';
require_once 'src/DrushRebuild/Rebuilder/Component/Variable.php';
require_once 'src/DrushRebuild/Rebuilder/Component/Permission.php';

/**
 * Implements hook_drush_help().
 */
function rebuild_drush_help($section) {
  switch ($section) {
    case 'meta:rebuild:title':
      return dt('Rebuild commands');
  }
}

/**
 * Implements hook_drush_command().
 */
function rebuild_drush_command() {
  // Set path to Drush Rebuild extension.
  $rebuild_dir = drush_server_home() . '/.drush/rebuild';

  $items['site-rebuild'] = array(
    'description' => 'Rebuilds a local Drupal environment.',
    'arguments' => array(),
    'options' => array(
      'source' => array(
        'description' => 'The drush alias to use as the source for the rebuild.',
        'example_value' => '@mysite.local',
      ),
      'dry-run' => array(
        'description' => 'List the Drush tasks that will be executed, but do not run them.',
      ),
      'view-config' => 'View the rebuild config.',
    ),
    'aliases' => array('rb', 'rebuild'),
    'examples' => array(
      'drush @mysite.local rebuild --source=@mysite.prod' => 'Rebuild the environment at @mysite.local using the source --@mysite.prod',
      'drush @mysite.local rebuild --view-config' => 'View the config for rebuilding @mysite.local. Does not execute any tasks.',
      'drush @mysite.local rebuild --dry-run' => 'List the Drush tasks that will be executed, but do not run them.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['site-rebuild-init'] = array(
    'description' => 'Create a rebuild config file in the current directory from a template.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'template' => 'The template to use for creating a rebuild file. Options are "remote" or "site-install".',
    ),
    'callback' => 'drush_rebuild_create',
  );

  $items['site-rebuild-error-log'] = array(
    'description' => 'View the errors from the last rebuild.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'outputformat' => array(
      'default' => 'yaml',
      'pipe-format' => 'json',
      'field-labels' => array('context' => 'Context'),
      'simplify-single' => TRUE,
      'table-metadata' => array(
        'list-separator' => ' ',
      ),
      'output-data-type' => 'format-list',
    ),
  );

  $items['site-rebuild-readme'] = array(
    'description' => 'Drush Rebuild README',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'aliases' => array('rebuild-docs'),
    'callback' => 'drush_print_file',
    'callback arguments' => array($rebuild_dir . '/README.md'),
  );

  $items['site-rebuild-example'] = array(
    'description' => 'Example Drush Rebuild file.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'callback' => 'drush_print_file',
    'callback arguments' => array($rebuild_dir . '/examples/example.rebuild.yaml'),
  );

  $items['site-rebuild-version'] = array(
    'description' => 'Show Drush Rebuild version.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['site-rebuild-info'] = array(
    'description' => 'Displays rebuild info for an alias.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'callback' => 'drush_rebuild_info',
    'aliases' => array('rbi'),
  );

  return $items;
}

/**
 * Display rebuild info for an alias.
 */
function drush_rebuild_info() {
  $alias = drush_get_context('alias');
  $drush_rebuild = new DrushRebuild($alias);
  $drush_rebuild->getInfo('@' . $alias['#name']);
}

/**
 * Displays the error log for a Drush alias.
 */
function drush_rebuild_site_rebuild_error_log() {
  $alias = drush_get_context('alias');
  if (!count($alias)) {
    return drush_set_error(dt('See usage for the command. You need to run this command with an alias.'));
  }
  $name = $alias['#name'];
  $log = drush_cache_get($name, 'rebuild-error-log');
  if (!$log) {
    return drush_set_error(dt('Could not find an error log for !alias', array('!alias' => $name)));
  }
  return $log->data;
}

/**
 * Create a rebuild file from a template.
 *
 * @param string $template
 *   The template to use.
 *
 * @return bool
 *   TRUE if successful, FALSE otherwise.
 */
function drush_rebuild_create($template = NULL) {
  $template_options = array('remote', 'site-install');
  if (!$template || !in_array($template, $template_options)) {
    return drush_set_error('You must use the "remote" or "site-install" template with this command.');
  }

  $config_filename = 'rebuild.yaml';

  $rebuild_dir = drush_server_home() . '/.drush/rebuild';
  $filename = '';
  switch ($template) {
    case 'remote':
      $filename = 'example.rebuild.yaml';
      break;

    case 'site-install':
      $filename = 'example.site_install.rebuild.yaml';
      break;
  }
  $current_dir = drush_cwd();
  $ret = copy($rebuild_dir . '/examples/' . $filename, $current_dir . '/' . $config_filename);
  if ($ret) {
    drush_log(dt('Successfully created a rebuild config file for you in !path', array('!path' => $current_dir . '/' . $config_filename)), 'success');
    return TRUE;
  }
  else {
    return drush_set_error(dt('Failed to create the rebuild config file for you.'));
  }
}

/**
 * Return the version number of Drush Rebuild.
 */
function _drush_rebuild_get_version() {
  $drush_info_file = dirname(__FILE__) . '/rebuild.info';
  $drush_rebuild_info = parse_ini_file($drush_info_file);
  return $drush_rebuild_info['version'];
}

/**
 * Rebuild a local development environment.
 *
 * @return bool
 *   TRUE on success; FALSE otherwise.
 */
function drush_rebuild_site_rebuild() {
  // Output version information and return.
  if (drush_get_option('version')) {
    return drush_print(dt("Drush Rebuild version: !version", array('!version' => _drush_rebuild_get_version())));
  }

  $alias = drush_get_context('alias');
  if (!count($alias)) {
    return drush_set_error(dt('Drush Rebuild requires the use of a Drush alias. Try running `drush @{site}.{local_environment} rebuild`.'));
  }
  $drush_rebuild = new DrushRebuild($alias);

  // Check if we are only viewing the script.
  if (drush_get_option('view-config')) {
    return $drush_rebuild->viewConfig();
  }

  // Set a flag if this is a dry run.
  if (drush_get_option('dry-run')) {
    $drush_rebuild->setDryRun();
  }

  // Check requirements.
  if (!$drush_rebuild->checkRequirements()) {
    return drush_set_error(dt('Requirements check failed for !target', array('!target' => $alias['#name'])));
  }

  // Load the rebuild config.
  $rebuild_config = $drush_rebuild->getConfig();
  if ($rebuild_config === FALSE) {
    return drush_set_error(dt('There was an error loading the rebuild config for !site', array('!site' => $alias['#name'])));
  }

  // Load metadata.
  $drush_rebuild->loadMetadata();
  // TODO: This should be done in checkRequirements.
  if (!isset($rebuild_config['site_install'])) {
    if (!$source = drush_get_option('source')) {
      // Check if there is a default source specified in the config.
      $source = isset($rebuild_config['sync']['default_source']) ? $rebuild_config['sync']['default_source'] : NULL;
    }
    if ($source && $drush_rebuild->isValidSource($source)) {
      $rebuild_config['sync']['source'] = $source;
      $drush_rebuild->setConfig($rebuild_config);
    }
    elseif (!isset($rebuild_config['sync'])) {
      // Neither remote sync nor site install are used. Assume that the user
      // has created a Drush script for rebuilding their site.
      drush_log("Neither 'sync' nor 'site_install' are defined in your rebuild config. Proceeding with the assumption that you are rebuilding in a custom script. If you did not expect to see this, then check your rebuild config file.", 'warning');
    }
    else {
      return drush_set_error(dt('There was no source environment specified for the rebuild!'));
    }
  }

  // Show rebuild meta-data for the alias.
  $drush_rebuild->showMetadata();

  if (drush_confirm(dt('Proceed with rebuilding the environment for !name', array('!name' => $alias['#name'])))) {
    $begin_rebuild_time = time();
    $ret = $drush_rebuild->rebuild();
    drush_print();
    if ($ret) {
      $completed_time = time();
      $total_rebuild_time = $completed_time - $begin_rebuild_time;
      // Update meta-data.
      $drush_rebuild->updateMetadata($total_rebuild_time);
      drush_log(dt('Rebuild completed in !min minute(s) and !sec second(s).',
        array(
          '!min' => gmdate("i", $total_rebuild_time),
          '!sec' => gmdate("s", $total_rebuild_time))
          ),
        'success'
      );
      return TRUE;
    }
    // Rebuild failed.
    return drush_log(dt('Rebuild failed to complete successfully.'), 'error');
  }
  // User cancelled the rebuild.
  drush_log(dt('Aborted rebuild for !name', array('!name' => $alias['#name'])), 'warning');
}
