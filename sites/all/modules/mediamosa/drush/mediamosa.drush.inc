<?php
/**
 * @file
 * Drush integration for MediaMosa.
 *
 * Contains common drush functions for MediaMosa.
 */

/**
 * Implements hook_drush_help().
 */
function mediamosa_drush_help($section) {
  switch ($section) {
    case 'drush:mediamosa-version':
      return dt('Returns the current MediaMosa version.');
  }
}

/**
 * Implements hook_drush_command().
 */
function mediamosa_drush_command() {
  $items = array();

  $items['mm-version'] = array(
    'description' => dt('Returns the current MediaMosa version.'),
    'aliases' => array('mmv'),
    'examples' => array(
      'mediamosa-version' => '',
    ),
  );

  $items['mm-client-app-add'] = array(
    'description' => 'Add a Client application.',
    'arguments' => array(
      'app_id' => 'Application id',
      'name' => 'Name of the new application',
      'password' => 'Password',
    ),
    'examples' => array(
      'drush mm-client-app-add 1 test changeme' => 'Add client id 1, with name test and password changeme.',
    ),
    'aliases' => array('mmapp'),
  );

  $items['mm-analyse'] = array(
    'description' => 'Analyse a mediafiles.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id',
    ),
    'options' => array(
      'priority' => 'Priority of the job, 0 = normal, -100 is maximum priority, 100 is lowest',
    ),
    'examples' => array(
      'drush mediamosa-analyse x2AQdmjOQNTUrEZnFEitPLwY' => 'Requests an analyse of a mediafile.',
      'drush mediamosa-analyse --priority=100 x2AQdmjOQNTUrEZnFEitPLwY' => 'The same, but with job priority.',
    ),
    'aliases' => array('mman'),
  );

  $items['mm-analyse-app'] = array(
    'description' => 'Analyse all mediafiles of an app.',
    'arguments' => array(
      'app_id' => 'App id',
    ),
    'options' => array(
      'priority' => 'Priority of the job, 0 = normal, -100 is maximum priority, 100 is lowest',
    ),
    'examples' => array(
      'drush mediamosa-analyse-app 1' => 'Requests to analyse all mediafiles of app id 1.',
      'drush mediamosa-analyse-app 1 --priority=10' => 'The same, but with lower priority.',
    ),
    'aliases' => array('mmanap'),
  );

  $items['mm-asset-list'] = array(
    'description' => 'Returns a list of assets.',
    'aliases' => array('mmal'),
  );

  $items['mm-asset'] = array(
    'description' => 'Returns details of an asset.',
    'arguments' => array(
      'asset_id' => 'Asset id',
    ),
    'aliases' => array('mmas'),
  );

  $items['mm-asset-mediafile-delete'] = array(
    'description' => 'Delete a mediafile.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id',
    ),
    'aliases' => array('mmamdel'),
  );

  $items['mm-asset-delete'] = array(
    'description' => 'Delete an asset and its mediafiles.',
    'arguments' => array(
      'asset_id' => 'Asset id',
    ),
    'aliases' => array('mmadel'),
  );

  $items['mm-app-purge'] = array(
    'description' => 'Deletes all assets from an app. Warning: really deletes all assets!',
    'arguments' => array(
      'app_id' => 'App id',
    ),
    'options' => array(
      'force' => 'Do not show a warning a proceed.',
    ),
  );

  $items['mm-apps-stats'] = array(
    'description' => 'Shows the number of assets and mediafiles of all apps.',
  );

  $items['mm-app-mf-list'] = array(
    'description' => 'Gives a list of all files including path of an application',
    'arguments' => array(
      'app_id' => 'App id',
    ),
  );

  $items['mm-generate-asset'] = array(
    'description' => dt('Generate asset(s). Make sure you run this as the webserver user (www-data, nginx).'),
    'arguments' => array(
      'number_assets' => dt('Number of assets to generate (default 1).'),
      'number_mediafiles' => dt('Number of mediafiles in the asset to generate (default 1).'),
      'app_id' => dt('Application id to store assets in (default 1).'),
      'file_path' => dt('file to use for mediafile (default lib/misc/mediamosa-logo.png).'),
    ),
    'aliases' => array('mmgen'),
    'options' => array(),
    'examples' => array(
      'drush mmgen 5 1 1 lib/misc/mediamosa-logo.png' => 'Generate 5 assets with a logo image.',
    ),
  );

  $items['mm-acl-set'] = array(
    'description' => 'Set ACL on a mediafile.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id',
      'acl' => 'acl rights (semicolon seperated)',
    ),
    'aliases' => array('mmacl'),
    'examples' => array(
      'drush mm-acl-set C1IVjPsFefqRNoOSV7jIbZtI foo.nl;bar.nl;@foo.nl' => 'Set the acl rights for domains foo.nl, bar.nl and for realms: @foo.nl.',
    ),
  );

  $items['mm-transcode'] = array(
    'description' => 'Start a transcode job.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id',
      'transcode_profile' => 'Transcode profile (number)',
    ),
    'aliases' => array('mmtr', 'mm-convert'),
    'examples' => array(
      'drush mmtr C1IVjPsFefqRNoOSV7jIbZtI 14' => 'Start a transcode or conversion job on mediafile id C1IVjPsFefqRNoOSV7jIbZtI with profile=14.',
    ),
  );

  $items['mm-retranscode'] = array(
    'description' => 'Start a retranscode job.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id',
    ),
    'aliases' => array('mmretr', 'mm-reconvert'),
    'options' => array(
      'priority' => 'Priority of the job, 0 = normal, -100 is maximum priority, 100 is lowest',
    ),
    'examples' => array(
      'drush mmretr C1IVjPsFefqRNoOSV7jIbZtI ' => 'Start a retranscode or reconversion job on mediafile id C1IVjPsFefqRNoOSV7jIbZtI.',
    ),
  );

  $items['mm-retranscode-app'] = array(
    'description' => 'Retranscode all mediafiles from an app with a transcode profile.',
    'arguments' => array(
      'app_id' => 'App id',
      'transcode_profile_id' => 'Transcode profile id',
    ),
    'aliases' => array('mmretrap', 'mm-reconvert-app'),
    'options' => array(
      'priority' => 'Priority of the job, 0 = normal, -100 is maximum priority, 100 is lowest',
    ),
    'examples' => array(
      'drush mmretrap 1 12 ' => 'Start a retranscode on all mediafiles with transcode 12 in appid 1.',
    ),
  );

  $items['mm-still'] = array(
    'description' => 'Start a still job.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id',
    ),
    'aliases' => array('mmstll'),
    'options' => array(
      'priority' => 'Priority of the job, 0 = normal, -100 is maximum priority, 100 is lowest',
    ),
    'examples' => array(
      'drush mm-still C1IVjPsFefqRNoOSV7jIbZtI ' => 'Request to generate a still from mediafile id C1IVjPsFefqRNoOSV7jIbZtI.',
    ),
  );

  $items['mm-jobs'] = array(
    'description' => 'Show a list of jobs in the queue.',
    'aliases' => array('mmjbs'),
    'examples' => array(
      'drush mm-jobs' => 'Give al list of all jobs in the queue',
    ),
  );

  $items['mm-job'] = array(
    'description' => 'Show job info.',
    'arguments' => array(
      'job_id' => 'Job id',
    ),
    'aliases' => array('mmjb'),
    'examples' => array(
      'drush mmjb 42' => 'Show all details of job with job_id = 42.',
    ),
  );

  $items['mm-job-cancel'] = array(
    'description' => 'Cancel a job.',
    'arguments' => array(
      'job_id' => 'Job id',
    ),
    'aliases' => array('mmjbc'),
    'examples' => array(
      'drush mmjbc 42' => 'Cancel Job with job_id = 42.',
    ),
  );

  $items['mm-asset-job-queue'] = array(
    'description' => 'Shows the number of jobs waiting of running of a given asset.',
    'arguments' => array(
      'asset_id' => 'Asset id',
    ),
    'aliases' => array('mmajq'),
    'examples' => array(
      'drush mmajq h2LMORcSojgPhPH3ch5cVXX6' => 'Show the number of running or waiting jobs of asset h2LMORcSojgPhPH3ch5cVXX6.',
    ),
  );

  $items['mm-import-directory'] = array(
    'description' => 'Imports the content of a directory in a new asset. If the directory contains a bag-info.txt, this file is used to add metadata to the asset.',
    'arguments' => array(
      'path' => 'Path to a directory that needs to be imported.',
    ),
    'options' => array(
      'symlink' => 'Use a symlink instead of a filecopy.',
      'quiet' => 'Suppress messages.',
    ),
    'aliases' => array('mmimp'),
    'examples' => array(
      'drush mmimp /mnt/media/item1' => 'Import the contents of directory /mnt/media/item1 into a new asset.',
      'drush mmimp --symlink /mnt/media/item1' => 'The same but now with symbolic links.',
    ),
  );

  return $items;
}

/**
 * Callback function for mm-version command.
 *
 * Returns the current MediaMosa version.
 */
function drush_mediamosa_mm_version() {
  drush_print(mediamosa_version::get_current_version_str(TRUE));
}

/**
 * Callback function for mm-client-app-add.
 */
function drush_mediamosa_mm_client_app_add($app_id, $name, $password) {

  if (!(is_numeric($app_id))) {
    return drush_set_error('', dt('Invalid app_id.'));
  }

  // Test if app_id already exists.
  if (mediamosa_app::get_by_appid($app_id)) {
    return drush_set_error('', dt('An app with this id already exists.'));
  }
  // Test if $name already exists.
  if (mediamosa_app::get_by_name($name)) {
    return drush_set_error('', dt('An app with this name already exists.'));
  }

  // We have 2 ways of doing this, or use the form api and drupal_form_submit,
  // or insert into the database directly. I prefer the first, however does not
  // work for unclear reasons. Feel free to improve here.
  /*
  $form_state = array();
  $form_state['values']['title'] = $name;
  $form_state['values']['app_id'] = $app_id;
  $form_state['values']['shared_key'] = $password;
  $form_state['values']['quota'] = 100;
  $form_state['submit_handlers'] = array('node_form_submit');
  drupal_form_submit('mediamosa_app_form', $form_state);
  print_r(form_get_errors());
  */

  // This just works.
  $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_APP, $name);
  $node->{mediamosa_app_db::APP_ID} = $app_id;
  $node->{mediamosa_app_db::SHARED_KEY} = $password;
  $node->{mediamosa_app_db::APP_NAME} = $name;
  $node->{mediamosa_app_db::ACTIVE} = 'TRUE';
  $node->{mediamosa_app_db::TRANSCODING_POSITION_TO} = 0;
  $node->{mediamosa_app_db::QUOTA} = 100;
  $node->{mediamosa_app_db::QUOTA_USED} = 0;
  $node->{mediamosa_app_db::ACTIVE_VERSION} = '';
  $node->{mediamosa_app_db::TIMEZONE} = '';
  $node->{mediamosa_app_db::OAUTH_CONSUMER_KEY} = $name;
  $node->{mediamosa_app_db::OAUTH_CONSUMER_SECRET} = user_password();
  node_save($node);
  drush_print("Application $name added.");
}

/**
 * Callback function for mm-asset-list command.
 *
 * Returns a list of assets.
 *
 * @param int $app_id
 *   Application id in which to search.
 */
function drush_mediamosa_mm_asset_list($app_id = 1) {

  if (!(is_numeric($app_id))) {
    return drush_set_error('', dt('Invalid app_id.'));
  }

  $limit = drush_get_option('limit', 25);
  $offset = drush_get_option('offset', 0);

  $assets = mediamosa_asset_search::asset_search(array($app_id), 'sortby asset_created/sort.descending', FALSE, NULL, array(), NULL, NULL, 0, 0, NULL, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, $limit, $offset);

  $rows[] = array('nr', 'Asset_id', 'Title', 'Created');
  foreach ($assets['asset_ids'] as $nr => $asset_id) {
    $asset_info = mediamosa_asset::get($asset_id, $app_id);
    $title = "";
    $metadata = mediamosa_asset_metadata::metadata_get($asset_id);
    if (isset($metadata['dublin_core']['title']['values'][0])) {
      $title = $metadata['dublin_core']['title']['values'][0];
    }
    $rows[] = array($nr, $asset_id, $title, $asset_info['created']);
  }
  drush_print_table($rows, TRUE);
  drush_print('Total: ' . $assets['total_count']);
}


/**
 * Callback function for mm-asset command.
 *
 * @param int $asset_id
 *   Asset id of which to give details.
 */
function drush_mediamosa_mm_asset($asset_id = NULL) {
  if ($asset_id == NULL) {
    // We get the last added asset op app_id.
    $assets = mediamosa_asset_search::asset_search(array(1), 'sortby asset_created/sort.descending', FALSE, NULL, array(), NULL, NULL, 0, 0, NULL, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, 1, 0);
    if (isset($assets['asset_ids'])) {
      $asset_id = $assets['asset_ids'][0];
    }
    else {
      return drush_set_error('', dt('Invalid asset_id.'));
    }
  }

  // Show asset details.
  $fields = array(
    mediamosa_asset_db::ID,
    mediamosa_asset_db::APP_ID,
    mediamosa_asset_db::OWNER_ID,
  );
  $asset = mediamosa_asset::get($asset_id, NULL, $fields);
  foreach ($asset as $name => $value) {
    $rows[] = array($name, $value);
  }
  $rows[] = array('');

  $metadata = mediamosa_asset_metadata::metadata_get($asset_id);
  foreach ($metadata as $metadata_name => $metadata_entry) {
    foreach ($metadata_entry as $name => $value) {
      foreach ($value['values'] as $val => $val_entry) {
        $rows[] = array($metadata_name . ':' . $name, $val_entry);
      }
    }
  }
  $rows[] = array('');

  // @todo: collections.
  // Show table of mediafiles.
  $rows[] = array('Mediafiles');
  $rows[] = array(
    'ID', 'Created', 'Profile id', 'Mime Type', 'FileSize', 'Size', 'Filename',
  );
  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::CREATED,
    mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID,
    mediamosa_asset_mediafile_db::FILENAME,
  );

  $mediafiles = mediamosa_asset_mediafile::get_by_asset_id($asset_id, $fields, array('orderby' => mediamosa_asset_mediafile_db::CREATED));

  foreach ($mediafiles as $mediafile_id => $mediafile) {
    $rows[] = array(
      $mediafile_id,
      $mediafile[mediamosa_asset_mediafile_db::CREATED],
      $mediafile[mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID],
      mediamosa_asset_mediafile::get_mimetype($mediafile_id),
      format_size(mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($mediafile_id, mediamosa_asset_mediafile_metadata::FILESIZE)),
      mediamosa_asset_mediafile::get_size($mediafile_id),
      $mediafile[mediamosa_asset_mediafile_db::FILENAME],
    );
  }
  drush_print_table($rows, TRUE);
}

/**
 * Callback function for mediamosa-mm-asset-mediafile-delete command.
 *
 * @param int $mediafile_id
 *   Mediafile id of the mediafile to delete.
 */
function drush_mediamosa_mm_asset_mediafile_delete($mediafile_id = NULL) {
  if ($mediafile_id == NULL) {
    return drush_set_error('', dt('Invalid mediafile_id.'));
  }
  mediamosa_asset_mediafile::delete($mediafile_id);
  drush_print('Mediafile id deleted: ' . $mediafile_id);
}

/**
 * Callback function for mediamosa-mm-acl-set command.
 *
 * @param int $mediafile_id
 *   Mediafile id of the file to set acl rights.
 */
function drush_mediamosa_mm_acl_set($mediafile_id = NULL, $acls = NULL) {
  if ($mediafile_id == NULL) {
    return drush_set_error('', dt('Invalid mediafile_id.'));
  }
  if (!mediamosa_asset_mediafile::must_exists($mediafile_id)) {
    return drush_set_error('', dt('mediafile_id does not exists.'));
  }

  // Get the media file.
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id);

  $app_ids = array();
  $a_acls = explode(',', $acls);
  $a_acl_realms = array();
  $a_acl_domains = array();
  foreach ($a_acls as $item) {
    if (strpos($item, '@') === FALSE) {
      // Domain.
      $a_acl_domains[$item] = $item;
    }
    else {
      // REALM.
      $a_acl_realms[$item] = $item;
    }
  }
  // Set access rights.
  $result_rights_set = mediamosa_acl::rights_set(
    $mediafile['app_id'], $mediafile['owner_id'], mediamosa_acl::ACL_TYPE_MEDIAFILE, $mediafile, $app_ids, array(), array(), $a_acl_domains, $a_acl_realms, 'replace');

  drush_print('Mediafile id has new acl: ' . print_r($result_rights_set, TRUE));
}

/**
 * Callback function for mediamosa-mm-asset-delete command.
 *
 * @param int $asset_id
 *   Asset id of the asset to delete.
 */
function drush_mediamosa_mm_asset_delete($asset_id = NULL) {
  if ($asset_id == NULL) {
    return drush_set_error('', dt('Invalid asset_id.'));
  }
  $asset = mediamosa_asset::delete($asset_id);
}

/**
 * Callback function for mediamosa-mm-app-purge command.
 *
 * @param int $app_id
 *   Application id of the application to clean.
 */
function drush_mediamosa_mm_app_purge($app_id = NULL) {
  if (!(is_numeric($app_id))) {
    return drush_set_error('', dt('Invalid app_id.'));
  }

  $force = drush_get_option('force', NULL);
  if (!$force) {
    if (!drush_confirm(dt('Are you sure?'))) {
      return drush_user_abort();
    }

    if (!drush_confirm(dt('Are you absolutely sure, with no doubt whatsoever in your mind, you want to do this?'))) {
      return drush_user_abort();
    }
  }
  $nbr_assets = db_query("SELECT count(asset_id) FROM {mediamosa_asset} where app_id = $app_id")->fetchField();
  drush_print('About to delete ' . $nbr_assets . ' assets from app_id: ' . $app_id);

  $assets = mediamosa_search::asset_search(array(
      'app_ids' => array($app_id),
      'limit' => 10000,
      'is_app_admin' => TRUE,
      'granted' => TRUE,
      'hide_empty_assets' => FALSE,
      )
  );

  $i = 0;
  $assets = db_query("SELECT asset_id FROM {mediamosa_asset} where app_id = $app_id");
  foreach ($assets as $asset) {
    mediamosa_asset::delete($asset->asset_id);
    show_status(++$i, $nbr_assets);
  }
  drush_print('Total deleted assets: ' . $i);
}

/**
 * Callback function for mediamosa-mm-apps-stats command.
 */
function drush_mediamosa_mm_apps_stats() {

  // Select app_id, app_name from mediamosa_app order by app_id;
  // select count(*) from mediamosa_asset where app_id = 1;
  // select count(*) from mediamosa_asset_mediafile where app_id = 1;
  $rows = array();

  $query = mediamosa_db::db_select('mediamosa_app', 'ma')
    ->fields('ma', array('app_id'))
    ->fields('ma', array('app_name'))
    ->orderBy('app_id');
  $apps = $query->execute()->fetchAllAssoc('app_id');

  $rows[0] = array('app_id', 'app name', '#assets', '#mediafiles');
  foreach ($apps as $app_id => $app) {
    $nbr_assets = db_query("SELECT count(asset_id) FROM {mediamosa_asset} where app_id = $app_id")->fetchField();
    $nbr_mediafiles = db_query("SELECT count(mediafile_id) FROM {mediamosa_asset_mediafile} where app_id = $app_id")->fetchField();
    $rows[] = array($app_id, $app['app_name'], $nbr_assets, $nbr_mediafiles);
  }

  drush_print_table($rows, TRUE);
}


/**
 * Callback function for mm-app-mf-list command.
 *
 * Returns a list of filenames (including sannas path)
 * of one app_id.
 *
 * @param int $app_id
 *   Id of the application..
 */
function drush_mediamosa_mm_app_mf_list($app_id = NULL) {
  if (!(is_numeric($app_id))) {
    return drush_set_error('', dt('Invalid app_id.'));
  }

  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
    ->fields('mf', array(mediamosa_asset_mediafile_db::ID))
    ->fields('mf', array(mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT))
    ->fields('mf', array(mediamosa_asset_mediafile_db::IS_STILL))
    ->condition(mediamosa_asset_mediafile_db::APP_ID, $app_id);
  $mediafiles = $query->execute()->fetchAllAssoc(mediamosa_asset_mediafile_db::ID);

  foreach ($mediafiles as $mediafile_id => $mediafile) {
    echo str_replace('//', '/', mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id));
    echo "\n";
  }
}


/**
 * Callback function for mm-analyse command.
 *
 * @param int $mediafile_id
 *   Mediafile id to analyse.
 * @param int $priority
 *   Job priority (-100 is high, 100 is low).
 */
function drush_mediamosa_mm_analyse($mediafile_id, $priority = 0) {

  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
    mediamosa_asset_mediafile_db::APP_ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
  );
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, $fields);

  $job = mediamosa_job::create_analyse_job(
    $mediafile['owner_id'],
    $mediafile['asset_id'],
    $mediafile['mediafile_id'],
    $mediafile['app_id'],
    $priority);

  $quiet = drush_get_option('quiet', NULL);
  if (!$quiet) {
    drush_print("Analyse job for mf: " . $mediafile['mediafile_id'] . " requested");
  }
}

/**
 * Callback function for mm-analyse-app command.
 *
 * @param int $app_id
 *   App id to analyse all mediafiles from.
 */
function drush_mediamosa_mm_analyse_app($app_id) {

  if (!(is_numeric($app_id))) {
    return drush_set_error('', dt('Invalid app_id.'));
  }
  $priority = drush_get_option('priority', 0);
  if (!(is_numeric($priority))) {
    return drush_set_error('', dt('Priority must be an integer.'));
  }

  $quiet = drush_get_option('quiet', NULL);

  $mm_fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
  );

  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
  $query->fields('mf', $mm_fields);
  $query->condition('mf.' . mediamosa_asset_mediafile_db::APP_ID, $app_id);
  $mediafiles = $query->execute()->fetchAllAssoc(mediamosa_asset_mediafile_db::ID);

  foreach ($mediafiles as $mediafile_id => $mediafile) {
    $job = mediamosa_job::create_analyse_job(
      $mediafile['owner_id'],
      $mediafile['asset_id'],
      $mediafile['mediafile_id'],
      $app_id,
      $priority);
    if (!$quiet) {
      drush_print("Analyse job for mf: " . $mediafile['mediafile_id'] . " requested");
    }
  }
}


/**
 * Callback function for mediamosa-generate-asset command.
 *
 * A command to generate random assets.
 *
 * @param int $number_assets
 *   Number of assets to generate.
 * @param int $number_mediafiles
 *   Number of mediafiles per asset.
 * @param int $app_id
 *   application id.
 */
function drush_mediamosa_mm_generate_asset($number_assets = 1, $number_mediafiles = 1, $app_id = 1, $file_path = NULL) {

  if (!is_numeric($number_assets)) {
    return drush_set_error('', dt('Invalid number of assets.'));
  }
  if (!is_numeric($number_mediafiles)) {
    return drush_set_error('', dt('Invalid number of mediafiles.'));
  }

  if ($file_path == NULL) {
    $file_path = DRUPAL_ROOT . '/' . drupal_get_path('module', 'mediamosa') . '/lib/misc/mediamosa-logo.png';
  }

  $title = drush_get_option('title', 'test asset #');

  for ($i = 0; $i < $number_assets; $i++) {
    $owner = 'N.N.' . $i;
    $asset = mediamosa_asset::create($app_id, $owner);
    drush_print('generated asset: ' . $asset);

    $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full(FALSE, array('dublin_core'));
    mediamosa_asset_metadata::metadata_create($asset, $metadata_definitions_full, array('title' => array($title . $i)), 'create');

    // mediafiles.
    for ($mf = 0; $mf < $number_mediafiles; $mf++) {
      $mediafile_id = mediamosa_asset_mediafile::create(
        mediamosa_db::uuid($app_id),
        $app_id,
        $asset,
        $owner,
        array(mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE));

      // filecopy.
      $mm_file_path = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id);
      $res = mediamosa_io::copy($file_path, $mm_file_path);
      if (!$res) {
        return drush_set_error('', dt('Filecopy failed, did you run this a webserver user?'));
      }

      // If root : change ownership to correct user:group.
      if (function_exists('posix_getuid')) {
        if (posix_getuid() == 0) {
          $sannas_info = mediamosa_configuration_storage::get_data_location();
          chown($mm_file_path, fileowner($sannas_info));
          chgrp($mm_file_path, filegroup($sannas_info));
        }
      }
      // Start analyse.
      drush_mediamosa_mm_analyse($mediafile_id);

      // Request still only for the first mediafile.
      if ($mf == 0) {
        mediamosa_job::create_job_still($app_id, $owner, TRUE, $mediafile_id, array());
      }
    }
  }

  drush_print('done.');
}

/**
 * Drush function mediamosa-mm-transcode.
 */
function drush_mediamosa_mm_transcode($mediafile_id, $transcode_profile = 1) {

  $priority = drush_get_option('priority', '0');

  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
    mediamosa_asset_mediafile_db::APP_ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
    mediamosa_asset_mediafile_db::GROUP_ID,
  );
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, $fields);
  if (!$mediafile) {
    return drush_set_error('', dt('Unknown mediafile_id.'));
  }

  $job_id = mediamosa_job::create_job_transcode(
    $mediafile['app_id'],
    $mediafile['owner_id'],
    $mediafile['group_id'],
    TRUE,
    $mediafile['mediafile_id'],
    FALSE, '', '', '', '', $transcode_profile, '',
    $priority);
  drush_print("Transcode job for mf: " . $mediafile['mediafile_id'] . " requested with profile id = " . $transcode_profile . ". Job id: " . $job_id . ".");
}

/**
 * Drush function mm-retranscode.
 */
function drush_mediamosa_mm_retranscode($mediafile_id) {

  $priority = drush_get_option('priority', '0');

  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
    mediamosa_asset_mediafile_db::APP_ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
    mediamosa_asset_mediafile_db::GROUP_ID,
  );
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, $fields);

  $job_id = mediamosa_job::create_job_retranscode(
    $mediafile['app_id'],
    $mediafile['owner_id'],
    $mediafile['group_id'],
    TRUE,
    $mediafile['mediafile_id'],
    FALSE, '', '', '', '', '', '',
    $priority);
  drush_print("RE-Transcode job for mf: " . $mediafile['mediafile_id'] . " requested. Job id: " . $job_id . ".");
}

/**
 * Drush function mm-retranscode-app.
 *
 * Retranscodes all mediafiles from a app with a transcode profile.
 */
function drush_mediamosa_mm_retranscode_app($app_id, $transcode_profile_id) {
  if (!is_numeric($app_id)) {
    return drush_set_error('', dt('Invalid app id.'));
  }
  if (!is_numeric($transcode_profile_id)) {
    return drush_set_error('', dt('Invalid transcode profile.'));
  }

  $priority = drush_get_option('priority', '0');

  $mm_fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
    mediamosa_asset_mediafile_db::GROUP_ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
  );

  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
  $query->fields('mf', $mm_fields);
  $query->condition('mf.' . mediamosa_asset_mediafile_db::APP_ID, $app_id);
  $query->condition('mf.' . mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID, $transcode_profile_id);
  $mediafiles = $query->execute()->fetchAllAssoc(mediamosa_asset_mediafile_db::ID);
  $count = 0;
  foreach ($mediafiles as $mediafile_id => $mediafile) {
    $job_id = mediamosa_job::create_job_retranscode(
    $app_id,
    $mediafile['owner_id'],
    $mediafile['group_id'],
    TRUE,
    $mediafile['mediafile_id'],
    FALSE, '', '', '', '', '', '',
    $priority);
    if (!$quiet) {
      drush_print("RE-Transcode job for mf: " . $mediafile['mediafile_id'] . " requested. Job id: " . $job_id . ".");
    }
    $count++;
  }
  drush_print(strtr("All done. @number retranscodes requested.", array('@number' => $count)));
}

/**
 * Drush function mm-still.
 *
 * Drush command to generate a still.
 */
function drush_mediamosa_mm_still($mediafile_id) {

  $priority = drush_get_option('priority', '0');

  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
    mediamosa_asset_mediafile_db::APP_ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
    mediamosa_asset_mediafile_db::GROUP_ID,
  );
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, $fields);

  // @todo: size from
  // mediamosa_app::get_still_default_value($mediafile['app_id']),
  $still_parameters = array(
    'still_type' => 'NONE',
    'still_per_mediafile' => 1,
    'frametime' => 24,
    'size' => '640x360',
    'tag' => NULL,
    'watermark_id' => NULL,
    'watermark_dst_x' => NULL,
    'watermark_dst_y' => NULL,
    'watermark_pct' => NULL,
    'watermark_v_align' => NULL,
    'watermark_h_align' => NULL,
  );

  $job_id = mediamosa_job::create_job_still(
    $mediafile['app_id'],
    $mediafile['owner_id'],
    TRUE,
    $mediafile['mediafile_id'],
    $still_parameters,
    $priority);
  drush_print("Still job for mf: " . $mediafile['mediafile_id'] . " requested. Job id: " . $job_id . ".");
}

/**
 * Drush function mm-jobs.
 *
 * Drush command which shows the current running and waiting jobs.
 */
function drush_mediamosa_mm_jobs() {

  $rows = array();
  $rows[0] = array('status', 'type', 'number');
  foreach (array('WAITING', 'INPROGRESS', 'FAILED', 'FINISHED') as $status) {
    foreach (array('ANALYSE', 'TRANSCODE', 'STILL') as $job_type) {
      $jobs = mediamosa_job::get_job_list(NULL, NULL, TRUE, 0, '', '', "status==\"$status\" and job_type==\"$job_type\"");
      $rows[] = array($job_type, $status, $jobs->found_rows);
    }
  }
  drush_print_table($rows);

  // In progress.
  $rows = array();
  $rows[0] = array(
    'job type',
    'job id',
    'asset_id',
    'started',
    'tool',
    'profile',
  );
  $jobs = mediamosa_job::get_job_list(NULL, NULL, TRUE, 0, '', '', 'status=="INPROGRESS"');
  foreach ($jobs as $job) {
    $job_info = mediamosa_job::get_job_ext($job['job_id']);
    $rows[] = array(
      $job['job_type'],
      $job['job_id'],
      $job['asset_id'],
      $job['started'],
      $job_info['tool'],
      $job_info['transcode_profile_id'],
      $job_info['command'],
      $job_info['progress'],
    );
  }
  if ($jobs->found_rows != 0) {
    drush_print_table($rows, TRUE);
  }
  else {
    drush_print("No current running jobs.");
  }
}

/**
 * Drush function mm-job.
 */
function drush_mediamosa_mm_job($job_id) {

  $fields = array(
    mediamosa_job_db::ID,
    mediamosa_job_db::ASSET_ID,
    mediamosa_job_db::MEDIAFILE_ID,
    mediamosa_job_db::APP_ID,
    mediamosa_job_db::OWNER_ID,
    mediamosa_job_db::JOB_STATUS,
    mediamosa_job_db::PROGRESS,
    mediamosa_job_db::JOB_TYPE,
    mediamosa_job_db::PRIORITY,
    mediamosa_job_db::CREATED,
    mediamosa_job_db::STARTED,
    mediamosa_job_db::FINISHED,
    mediamosa_job_db::ERROR_DESCRIPTION,
  );

  $job = mediamosa_job::get($job_id, $fields);

  drush_print("Job details: " . print_r($job, TRUE) . ".");
}

/**
 * Drush function mm-job-cancel.
 *
 * Drush command to cancel a job.
 *
 * @param int $job_id
 *   Job id of the job to cancel.
 */
function drush_mediamosa_mm_job_cancel($job_id) {

  $fields = array(
    mediamosa_job_db::APP_ID,
    mediamosa_job_db::OWNER_ID,
  );

  $job = mediamosa_job::get($job_id, $fields);

  mediamosa_job::cancel_job($job[mediamosa_job_db::APP_ID], $job_id, $job[mediamosa_job_db::OWNER_ID]);

  drush_print("Job cancel request done: " . print_r($job, TRUE) . ".");
}

/**
 * Drush function mm-asset-job-queue.
 *
 * Drush command which shows all jobs of a given asset.
 *
 * @param string $asset_id
 *   Asset id.
 */
function drush_mediamosa_mm_asset_job_queue($asset_id) {

  $rows = array();

  if ($asset_id == NULL) {
    return drush_set_error('', dt('Invalid asset_id.'));
  }

  $jobs = mediamosa_job::get_by_asset_id($asset_id);
  foreach ($jobs as $job) {
    $job_info = mediamosa_job::get_job_ext($job['job_id']);
    $rows[] = array(
      $job['job_type'],
      $job['status'],
      $job['job_id'],
      $job['asset_id'],
      $job['started'],
      $job_info['tool'],
      $job_info['transcode_profile_id'],
      $job_info['command'],
      $job_info['progress'],
    );
  }
  if (count($rows) > 0) {
    drush_print_table($rows, TRUE);
  }
  else {
    drush_print("No jobs for this asset (yet).");
  }
}

/**
 * Helper function to parse a bagit-info.txt.
 */
function _drush_mediamosa_parse_bagit_info($header) {
  $ret = array();
  $fields = explode("\n", $header);

  foreach ($fields as $field) {
    if (preg_match('/([^:]+): (.+)/m', $field, $match)) {
      $ret[$match[1]][] = $match[2];
      $last_match = $match[1];
    }
    else {
      if ($field != '') {
        if ($last_match != '') {
          $last_item = count($ret[$last_match]);
          if ($last_item > 0) {
            $ret[$last_match][$last_item - 1] .= "\n" . ltrim($field);
          }
        }
      }
    }
  }
  return $ret;
}

/**
 * Helper function to handle non standard metadata definitions.
 */
function _mediamosa_bagit_metadata_convert($tag_name, $to_bagit = TRUE) {
  $conversions = array(
    'DC-Title' => 'title',
    'DC-AccessRights' => 'rights',
    'DC-Creator' => 'creator',
    'DC-DateAccepted' => 'date',
    'DC-Description' => 'description',
    'DC-Identifier' => 'identifier',
    'DC-Type' => 'type',
  );
  if ($to_bagit) {
    // Convert from MediaMosa to BagIt.
    return (array_search($tag_name, $conversions));
  }
  else {
    // Convert from BagIt to MediaMosa.
    if (isset($conversions[$tag_name])) {
      return ($conversions[$tag_name]);
    }
  }
  return $tag_name;
}

/**
 * Drush function mm-import-directory.
 */
function drush_mediamosa_mm_import_directory($path, $app_id = 1, $owner = 'n.n.') {

  // Sanitize path.
  $path .= (drupal_substr($path, -1) == '/' ? '' : '/');

  $symlink = drush_get_option('symlink', NULL);
  $quiet = drush_get_option('quiet', NULL);

  if (count(scandir($path)) == 2) {
    if (!$quiet) {
      drush_print("No files found in directory.");
    }
    else {
      return FALSE;
    }
  }

  // Create asset id.
  // @todo: don't put path in group_id.
  $asset_id = mediamosa_asset::create($app_id, $owner, $path);

  // Add metadata from a bag-info.txt file.
  $file_baginfo = $path . 'bag-info.txt';
  $metadata = array();
  if (file_exists($file_baginfo)) {
    $baginfo = _drush_mediamosa_parse_bagit_info(file_get_contents($file_baginfo));

    foreach ($baginfo as $property => $value) {
      // Convert BagIt metadata to MediaMosa metadata.
      $property = _mediamosa_bagit_metadata_convert($property, FALSE);
      // @todo: handle unknown metadata in MediaMosa, such as Archive-id.
      $metadata[$property] = $value;
    }

    $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full(FALSE, array('dublin_core'));
    mediamosa_asset_metadata::metadata_create($asset_id, $metadata_definitions_full, $metadata, 'create');
  }

  // Loop directory recursively, to add files to the asset.
  $objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($path), RecursiveIteratorIterator::SELF_FIRST);
  foreach ($objects as $name => $object) {
    if ($object->isFile()) {

      // Add mediafile id and store relavative pathname in metadata.
      $mediafile_id = mediamosa_asset_mediafile::create(
        mediamosa_db::uuid($app_id),
        $app_id,
        $asset_id,
        $owner,
        array(
          // Objects, as the object does not have a relative sub path name.
          mediamosa_asset_mediafile_db::FILENAME => $objects->getsubPathName(),
          mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE));

      // filecopy.
      $mm_file_path = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id);

      if ($symlink != NULL) {
        $res = mediamosa_io::symlink($object->getPathName(), $mm_file_path);
      }
      else {
        $res = mediamosa_io::copy($object->getPathName(), $mm_file_path);
      }
      if (!$res) {
        return drush_set_error('', dt('Filecopy failed, did you run this as a webserver user?'));
      }

      // If root : change ownership to correct user:group.
      if (function_exists('posix_getuid')) {
        if (posix_getuid() == 0) {
          $sannas_info = mediamosa_configuration_storage::get_data_location();
          chown($mm_file_path, fileowner($sannas_info));
          chgrp($mm_file_path, filegroup($sannas_info));
        }
      }
      // Start analyse.
      drush_mediamosa_mm_analyse($mediafile_id);
    }
  }
  if (!$quiet) {
    drush_print("import done. New asset_id: $asset_id");
  }

  // Done.
  return ($asset_id);
}

/*
Copyright (c) 2010, dealnews.com, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of dealnews.com, Inc. nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

 */

/**
 * Show a status bar in the console.
 *
 * <code>
 * for($x=1;$x<=100;$x++){
 *
 *     show_status($x, 100);
 *
 *     usleep(100000);
 *
 * }
 * </code>
 *
 * @param int $done
 *   how many items are completed
 * @param int $total
 *   how many items are to be done total
 * @param int $size
 *   optional size of the status bar
 *
 * @return void
 *   void.
 */
function show_status($done, $total, $size = 30) {

  static $start_time;

  // If we go over our bound, just ignore it.
  if ($done > $total) {
    return;
  }

  // Only by steps of 10.
  if (($done < $total) && (($done % 10) != 0)) {
    return;
  }

  if (empty($start_time)) {
    $start_time = time();
  }
  $now = time();

  $perc = (double) ($done / $total);

  $bar = floor($perc * $size);

  $status_bar = "\r[";
  $status_bar .= str_repeat("=", $bar);
  if ($bar < $size) {
    $status_bar .= ">";
    $status_bar .= str_repeat(" ", $size - $bar);
  }
  else {
    $status_bar .= "=";
  }

  $disp = number_format($perc * 100, 0);

  $status_bar .= "] $disp%  $done/$total";

  $rate = ($now - $start_time) / $done;
  $left = $total - $done;
  $eta = round($rate * $left, 2);

  $elapsed = $now - $start_time;

  $status_bar .= " remaining: " . number_format($eta) . " sec.  elapsed: " . number_format($elapsed) . " sec.";

  echo "$status_bar  ";

  flush();
  // When done, send a newline.
  if ($done == $total) {
    echo "\n";
  }
}
