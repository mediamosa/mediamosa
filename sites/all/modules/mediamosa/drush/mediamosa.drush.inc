<?php
/**
 * @file
 * Drush integration for MediaMosa.
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
    'bootstrap' => 'DRUSH_BOOTSTRAP_DRUPAL_LOGIN',
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
