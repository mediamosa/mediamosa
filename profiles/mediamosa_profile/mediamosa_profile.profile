<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * MediaMosa installation profile.
 */

/**
 * @defgroup constants Module Constants
 * @{
 */

/**
 * Test text for Lua Lpeg
 */
define('MEDIAMOSA_PROFILE_TEST_LUA_LPEG', 'lua works');

/**
 * Retrieve the version.
 */
function _mediamosa_profile_get_version() {
  $inc = include_once (DRUPAL_ROOT . '/sites/all/modules/mediamosa/mediamosa.version.class.inc');

  // Try to include the settings file.
  return $inc ? mediamosa_version::get_current_version_str(TRUE) : '';
}

/**
 * Set up title.
 */
function _mediamosa_profile_get_title() {
  return 'Installing MediaMosa ' . _mediamosa_profile_get_version();
}

/**
 * Implementation of hook_install_tasks().
 */
function mediamosa_profile_install_tasks() {

  drupal_set_title(_mediamosa_profile_get_title());

  $tasks = array(
    'mediamosa_profile_storage_location_form' => array(
      'display_name' => st('Storage location'),
      'type' => 'form',
      'run' => variable_get('mediamosa_current_mount_point', '') ? INSTALL_TASK_SKIP : INSTALL_TASK_RUN_IF_NOT_COMPLETED,
    ),
    'mediamosa_profile_configure_server' => array(
      'display_name' => st('Configure the server'),
    ),
    'mediamosa_profile_apache_settings_form' => array(
      'display_name' => st('Apache settings'),
      'type' => 'form',
    ),
    'mediamosa_profile_domain_usage_form' => array(
      'display_name' => st('Your domain usage'),
      'type' => 'form',
      'run' => variable_get('apache_setting') == 'simple' ? INSTALL_TASK_SKIP : INSTALL_TASK_RUN_IF_NOT_COMPLETED,
    ),
    'mediamosa_profile_migration_form' => array(
      'display_name' => st('Migration of your v1.7 database'),
      'type' => 'form',
    ),
    'mediamosa_profile_cron_settings_form' => array(
      'display_name' => st('Cron settings'),
      'type' => 'form',
    ),
  );
  return $tasks;
}

/**
 * Implementation of hook_install_tasks_alter().
 */
function mediamosa_profile_install_tasks_alter(&$tasks, $install_state) {

  // We need to rebuild tasks in the same order and put our
  // 'mediamosa_profile_php_settings_form' between it.
  $copy_tasks_till = 'install_bootstrap_full';

  $tasks_rebuild = array();

  $mediamosa_profile_php_settings_form = array(
    'display_name' => st('MediaMosa requirements'),
    'type' => 'form',
  );

  foreach ($tasks as $name => $task) {
    $tasks_rebuild[$name] = $task; // Copy task.

    // if we reach certain point, then insert our task.
    if ($name == $copy_tasks_till) {
      $tasks_rebuild['mediamosa_profile_php_settings_form'] = $mediamosa_profile_php_settings_form;
    }
  }

  // Copy rebuild.
  $tasks = $tasks_rebuild;
}

function system_form_install_settings_form_alter(&$form, $form_state, $form_id) {
  // Set default for site name field.
  $form['intro'] = array(
    '#weight' => -1,
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t("Set up Database"),
    '#description' => t("
    <p>We advice using !mysql v5.1, or use MySQL variant like !mariadb.
    MediaMosa is currently <b>untested</b> with !postgresql.
   </p>
   <p>Use the database <b>mediamosa</b> example below to create your database 'mediamosa' with user 'mediamosa' before proceeding.</p>
    <code>
        # The password entries below needs to be changed.<br />
        <br />
        # Create the database.<br />
        CREATE DATABASE mediamosa DEFAULT CHARSET=utf8;<br />
        <br />
        # Create localhost access for user 'mediamosa'.<br />
        CREATE USER 'mediamosa'@'localhost' IDENTIFIED BY 'mediamosa';<br />
        <br />
        # Now grant usage for user 'mediamosa' on the 'mediamosa' database.<br />
        GRANT USAGE ON mediamosa.* TO 'mediamosa'@'localhost' IDENTIFIED BY 'mediamosa' WITH MAX_QUERIES_PER_HOUR 0 MAX_CONNECTIONS_PER_HOUR 0 MAX_UPDATES_PER_HOUR 0 MAX_USER_CONNECTIONS 0;<br />
        <br />
        GRANT ALL ON mediamosa.* TO 'mediamosa'@'localhost';<br />
    </code>
    <p>
        You may change the 'mediamosa' database prefix and the database user name.<br />
        <br />
        If you want to migrate your current MediaMosa v1.7 database to the new 3.x version, you have to create or have a database user, which has enough rights to read your current v1.7 databases.</p>
   ", array(
    '!mysql' => l('MySQL', 'http://mysql.com/'),
    '!mariadb' => l('MariaDB', 'http://mariadb.org/'),
    '!postgresql' => l('PostgreSQL', 'http://www.postgresql.org/')
   ))
  );
}

/**
 * Implementation of hook_form_alter().
 */
function system_form_install_configure_form_alter(&$form, $form_state, $form_id) {
  $form['site_information']['site_name']['#default_value'] = 'MediaMosa';
  $form['site_information']['site_mail']['#default_value'] = 'webmaster@' . $_SERVER['SERVER_NAME'];
  $form['admin_account']['account']['name']['#default_value'] = 'admin';
  $form['admin_account']['account']['mail']['#default_value'] = 'admin@' . $_SERVER['SERVER_NAME'];
}

function mediamosa_profile_php_settings_form($form, &$form_state, &$install_state) {

  $php_modules = _mediamosa_profile_php_modules();
  $installed_programs = _mediamosa_profile_installed_programs();
  $php_settings = _mediamosa_profile_php_settings();
  $errors = $php_modules['errors'] + $installed_programs['errors'] + $php_settings['errors'];

  $form['requirements']['php_modules']['title'] = array(
    '#markup' => '<h1>' . t('PHP Modules') . '</h1>'
  );
  $form['requirements']['php_modules']['requirements'] = array(
    '#markup' => theme('status_report', array('requirements' => $php_modules['requirements']))
  );

  $form['requirements']['installed_programs']['title'] = array(
    '#markup' => '<h1>' . t('Installed programs') . '</h1>'
  );
  $form['requirements']['installed_programs']['requirements'] = array(
    '#markup' => theme('status_report', array('requirements' => $installed_programs['requirements']))
  );

  $form['requirements']['php_settings']['title'] = array(
    '#markup' => '<h1>' . t('PHP variables / Settings') . '</h1>'
  );
  $form['requirements']['php_settings']['requirements'] = array(
    '#markup' => theme('status_report', array('requirements' => $php_settings['requirements']))
  );

  if ($errors) {
    $form['requirements']['errors']['text'] = array(
      '#markup' => "<p><b>Fix reported problems and press 'continue' to continue.</b></p>"
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => st('Continue'),
  );

  return $form;

}

/**
 * Validate.
 */
function mediamosa_profile_php_settings_form_validate($form, &$form_state) {
  $php_modules = _mediamosa_profile_php_modules();
  $installed_programs = _mediamosa_profile_installed_programs();
  $php_settings = _mediamosa_profile_php_settings();

  $errors = $php_modules['errors'] + $installed_programs['errors'] + $php_settings['errors'];

  if ($errors) {
    form_set_error('foo', st('Fix the reported problems below before you continue. You can ignore the (yellow) warnings.'));
  }
}

/**
 * Submit the intro form.
 */
function mediamosa_profile_php_settings_form_submit($form, &$form_state) {

}

/**
 * Checking the php modules.
 */
function _mediamosa_profile_php_modules() {

  $errors = 0;
  $requirements = array();
  // title, value, description (op), severity (op)

  // Required modules.
  $required_extensions = array('bcmath', 'gd', 'curl', 'mysql', 'mysqli', 'SimpleXML');

  $loaded_extensions = get_loaded_extensions();
  foreach ($required_extensions as $extension) {
    $missing = !in_array($extension, $loaded_extensions);

    $requirements[$extension] = array(
      'title' => st('<b>PHP module ' . $extension . ':</b>'),
      'value' => !$missing ? 'Installed' : 'PHP module ' . $extension . ' is not installed.' ,
      'severity' => !$missing ? REQUIREMENT_OK : REQUIREMENT_ERROR,
    );
  }
  $exec_output = array();

  $errors = 0;
  foreach ($requirements as $requirement) {
    if ($requirement['severity'] == REQUIREMENT_ERROR || $requirement['severity'] == REQUIREMENT_WARNING) {
      $errors++;
    }
  }

  return array('errors' => $errors, 'requirements' => $requirements);
}

/**
 * Checking the installed programs.
 */
function _mediamosa_profile_installed_programs() {

  // Just here for notice prev.
  $ret_val = 0;

  // FFmpeg.
  $exec_output = array();
  exec('ffmpeg -version > /dev/null 2>&1', $exec_output, $ret_val);
  $requirements['ffmpeg'] = array(
    'title' => st('<b>Program FFmpeg:</b>'),
    'value' => !$ret_val ? 'Installed' : 'FFmpeg is not installed.' ,
    'severity' => !$ret_val ? REQUIREMENT_OK : REQUIREMENT_ERROR,
    'description' => !$ret_val ? '' : st('Install !ffmpeg.', array('!ffmpeg' => l('FFmpeg', 'http://www.ffmpeg.org/', array('attributes' => array('target' => '_blank'), 'absolute' => TRUE, 'external' => TRUE)))),
  );

  // Lua.
  $exec_output = array();
  exec('lua 2>&1', $exec_output, $ret_val);
  $requirements['lua'] = array(
    'title' => st('<b>Program LUA 5.1:</b>'),
    'value' => !$ret_val ? 'Installed' : 'LUA is not installed.' ,
    'severity' => !$ret_val ? REQUIREMENT_OK : REQUIREMENT_ERROR,
    'description' => !$ret_val ? '' : st('Install LUA 5.1. You can find more information how to install LUA !here', array('!here' => l('here', 'http://mediamosa.org/forum/viewtopic.php', array('attributes' => array('target' => '_blank'), 'absolute' => TRUE, 'external' => TRUE, 'query' => array('f'=> '13', 't' => '175', 'start' => '10'), 'fragment' => 'p687')))),
  );

  // Lpeg.
  $exec_output = array();
  exec('lua ' . escapeshellcmd(DRUPAL_ROOT) . '/profiles/mediamosa_profile/lua/lua_test 2>&1', $exec_output, $ret_val);

  $requirements['lpeg'] = array(
    'title' => st('<b>LUA extension Lpeg:</b>'),
    'value' => !$ret_val ? 'Installed' : 'Lpeg extension is not installed.' ,
    'severity' => !$ret_val ? REQUIREMENT_OK : REQUIREMENT_ERROR,
    'description' => !$ret_val ? '' : st('Install Lpeg extension for LUA. You can find more information how to install Lpeg !here', array('!here' => l('here', 'http://mediamosa.org/forum/viewtopic.php', array('attributes' => array('target' => '_blank'), 'absolute' => TRUE, 'external' => TRUE, 'query' => array('f'=> '13', 't' => '175', 'start' => '10'), 'fragment' => 'p687')))),
  );

  $errors = 0;
  foreach ($requirements as $requirement) {
    if ($requirement['severity'] == REQUIREMENT_ERROR || $requirement['severity'] == REQUIREMENT_WARNING) {
      $errors++;
    }
  }

  return array('errors' => $errors, 'requirements' => $requirements);
}

/**
 * Checking the PHP Settings.
 *
 * Only possible warnings for now.
 */
function _mediamosa_profile_php_settings() {

  $php_upload_max_filesize = ini_get('upload_max_filesize');
  $too_low = (substr($php_upload_max_filesize, 0, -1) < 100) && (substr($php_upload_max_filesize, -1) != 'M' || substr($php_upload_max_filesize, -1) != 'G');
  $requirements['upload_max_filesize'] = array(
    'title' => st('<b>upload_max_filesize:</b>'),
    'value' => $php_upload_max_filesize,
    'severity' => !$too_low ? REQUIREMENT_OK : REQUIREMENT_WARNING,
    'description' => !$too_low ? '' : st('Warning: upload_max_filesize should be at least 100M.'),
  );

  $php_memory_limit = ini_get('memory_limit');
  $too_low = (substr($php_memory_limit, 0, -1) < 128) && (substr($php_memory_limit, -1) != 'M' || substr($php_memory_limit, -1) != 'G');
  $requirements['memory_limit'] = array(
    'title' => st('<b>memory_limit:</b>'),
    'value' => $php_memory_limit,
    'severity' => !$too_low ? REQUIREMENT_OK : REQUIREMENT_WARNING,
    'description' => !$too_low ? '' : st('Warning: memory_limit should be at least 128M.'),
  );

  $php_post_max_size = ini_get('post_max_size');
  $too_low = (substr($php_post_max_size, 0, -1) < 100) && (substr($php_post_max_size, -1) != 'M' || substr($php_post_max_size, -1) != 'G');
  $requirements['php_post_max'] = array(
    'title' => st('<b>post_max_size:</b>'),
    'value' => $php_post_max_size,
    'severity' => !$too_low ? REQUIREMENT_OK : REQUIREMENT_WARNING,
    'description' => !$too_low ? '' : st('Warning: post_max_size should be at least 100M.'),
  );

  return array('errors' => 0, 'requirements' => $requirements);
}

/**
 * Get the mount point.
 * Task callback.
 */
function mediamosa_profile_storage_location_form() {
  $form = array();

  $mount_point = variable_get('mediamosa_current_mount_point', '/srv/mediamosa');
  $mount_point_windows = variable_get('mediamosa_current_mount_point_windows', '\\\\');

  $form['description'] = array(
    '#markup' => '<p><b>' . st('The mount point is a shared directory where related mediafiles, images and other files are stored. On a multi-server setup, this mount point needs to be available for all servers (i.e. through NFS)') . '</b></p>',
  );

  $form['current_mount_point'] = array(
    '#type' => 'textfield',
    '#title' => t('MediaMosa SAN/NAS Mount point'),
    '#description' => t('Make sure the Apache user has write access to the MediaMosa SAN/NAS mount point.'),
    '#required' => TRUE,
    '#default_value' => $mount_point,
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => st('Continue'),
  );

  return $form;
}

function mediamosa_profile_storage_location_form_validate($form, &$form_state) {
  $values = $form_state['values'];

  if (trim($values['current_mount_point']) == '') {
    form_set_error('current_mount_point', t("The current Linux mount point can't be empty."));
  }
  elseif (!is_dir($values['current_mount_point'])) {
    form_set_error('current_mount_point', t('The current Linux mount point is not a directory.'));
  }
  elseif (!is_writable($values['current_mount_point'])) {
    form_set_error('current_mount_point', t('The current Linux mount point is not writeable for the apache user.'));
  }
}

function mediamosa_profile_storage_location_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  variable_set('mediamosa_current_mount_point', $values['current_mount_point']);
  variable_set('mediamosa_current_mount_point_windows', '\\');

  // Inside the storage location, create a MediaMosa storage structure.
  // data.
  _mediamosa_profile_mkdir($values['current_mount_point'], '/data');
  for ($i = 0; $i <= 9; $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'], '/data/' . $i);
  }
  for ($i = ord('a'); $i <= ord('z'); $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'], '/data/' . chr($i));
  }
  for ($i = ord('A'); $i <= ord('Z'); $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'], '/data/' . chr($i));
  }
  // data/stills.
  _mediamosa_profile_mkdir($values['current_mount_point'], '/data/stills');
  for ($i = 0; $i <= 9; $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'], '/data/stills/' . $i);
  }
  for ($i = ord('a'); $i <= ord('z'); $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'], '/data/stills/' . chr($i));
  }
  for ($i = ord('A'); $i <= ord('Z'); $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'], '/data/stills/' . chr($i));
  }
  // Other.
  _mediamosa_profile_mkdir($values['current_mount_point'], '/data/transcode');
  _mediamosa_profile_mkdir($values['current_mount_point'], '/links');
  _mediamosa_profile_mkdir($values['current_mount_point'], '/download_links');
  _mediamosa_profile_mkdir($values['current_mount_point'], '/still_links');
  _mediamosa_profile_mkdir($values['current_mount_point'], '/ftp');
}

/**
 * Configure the server.
 * Tasks callback.
 */
function mediamosa_profile_configure_server($install_state) {
  $output = '';
  $error = FALSE;

  $server_name = _mediamosa_profile_server_name();


  // Configure the servers.

  // MediaMosa server table.
  db_query("
    UPDATE {mediamosa_server}
    SET server_uri = REPLACE(server_uri, 'mediamosa.local', :server)
    WHERE LOCATE('mediamosa.local', server_uri) > 0", array(
    ':server' => $server_name,
  ));
  db_query("
    UPDATE {mediamosa_server}
    SET uri_upload_progress = REPLACE(uri_upload_progress, 'mediamosa.local', :server)
    WHERE LOCATE('mediamosa.local', uri_upload_progress) > 0", array(
    ':server' => $server_name,
  ));
  db_query("
    UPDATE {mediamosa_server}
    SET uri_upload_progress_server = REPLACE(uri_upload_progress_server, 'mediamosa.local', :server)
    WHERE LOCATE('mediamosa.local', uri_upload_progress_server) > 0", array(
    ':server' => $server_name,
  ));

  // MediaMosa node revision table.
  $result = db_query("SELECT nid, vid, revision_data FROM {mediamosa_node_revision}");
  foreach ($result as $record) {
    $revision_data = unserialize($record->revision_data);
    if (isset($revision_data['server_uri'])) {
      $revision_data['server_uri'] = str_replace('mediamosa.local', $server_name, $revision_data['server_uri']);
    }
    if (isset($revision_data['uri_upload_progress'])) {
      $revision_data['uri_upload_progress'] = str_replace('mediamosa.local', $server_name, $revision_data['uri_upload_progress']);
    }
    if (isset($revision_data['uri_upload_progress_server'])) {
      $revision_data['uri_upload_progress_server'] = str_replace('mediamosa.local', $server_name, $revision_data['uri_upload_progress_server']);
    }
    db_query("
      UPDATE {mediamosa_node_revision}
      SET revision_data = :revision_data
      WHERE nid = :nid AND vid = :vid", array(
      ':revision_data' => serialize($revision_data),
      ':nid' => $record->nid,
      ':vid' => $record->vid,
    ));
  }


  // Configure.
  // URL REST.
  variable_set('mediamosa_cron_url_app', 'http://app1.' . $server_name . (substr($server_name, -6) == '.local' ? '' : '.local'));

  // Configure mediamosa connector.
  variable_set('mediamosa_connector_url', 'http://' . $server_name);
  $result = db_query("SELECT app_name, shared_key FROM {mediamosa_app} LIMIT 1");
  foreach ($result as $record) {
    variable_set('mediamosa_connector_username', $record->app_name);
    variable_set('mediamosa_connector_password', $record->shared_key);
  }


  return $error ? $output : NULL;
}

/**
 * Information about cron, apache and migration.
 * Task callback.
 */
function mediamosa_profile_cron_settings_form() {
  $form = array();

  // Add our css.
  drupal_add_css('profiles/mediamosa_profile/mediamosa_profile.css');

  // Get the server name.
  $server_name = _mediamosa_profile_server_name();
  if (variable_get('apache_setting') == 'simple') {
    $server_name = 'localhost';
  }

  // Cron.
  $form['cron'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Cron setup'),
    '#description' => t('The cron will be used trigger MediaMosa every minute for background jobs. The setup for cron is required for MediaMosa to run properly.'),
  );

  $form['cron']['crontab_text'] = array(
    '#markup' => t('<h5>Add a crontab entry</h5>Modify your cron using crontab, this will run the script every minute:<p><code>crontab -e</code></p><p>Add this line at the bottom:</p>'),
  );

  $form['cron']['crontab'] = array(
    '#markup' => '<p><pre>* * * * * /usr/bin/wget -O - -q -t 1 --header="Host: ' . $server_name . '" http://localhost' . url('') . 'cron.php?cron_key=' . variable_get('cron_key', '') . '</pre></p>',
    '#rows' => 6,
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  // Flush all.
  drupal_flush_all_caches();

  return $form;
}

/**
 * Information about cron, apache and migration.
 * Task callback.
 */
function mediamosa_profile_apache_settings_form() {
  $form = array();

  // Add our css.
  drupal_add_css('profiles/mediamosa_profile/mediamosa_profile.css');

  // Get the server name.
  $server_name = _mediamosa_profile_server_name();
  $mount_point = variable_get('mediamosa_current_mount_point', '');
  $document_root = _mediamosa_profile_document_root();

  $apache_settings_local = st("Single-server setup with http://localhost/ for demonstration or testing purposes.
  <p><li>Add the following lines to your default localhost apache definition:</p>
    <pre>" . htmlentities("
    # ticket
    Alias /ticket !mount_point/links
    <Directory !mount_point/links>
      Options FollowSymLinks
      AllowOverride All
      Order deny,allow
      Allow from All
    </Directory>

    <IfModule mod_php5.c>
        php_admin_value post_max_size 2008M
        php_admin_value upload_max_filesize 2000M
        php_admin_value memory_limit 128M
    </IfModule>") . '</pre>
<p>The ticket is the streaming link to a video needed to play videos, the php settings are needed to allow more than default sizes upload.</p>
<p><li>Restart your Apache:</p><p><code>sudo /etc/init.d/apache2 restart</code></p>
', array(
      '!mount_point' => $mount_point,
    ));

  $server_name_clean = $server_name;

  $apache_settings_adv = st("Multi-server setup with different DNS entries for a production or development setup.
<p><li>Insert the vhost setup below into the new file /etc/apache2/sites-available/<b>your-site</b>, where <b>your-site</b> is the name of your MediaMosa site:</p>
<pre>" . htmlentities("
<VirtualHost *:80>
    ServerName !server_name_clean
    ServerAlias admin.!server_name_clean www.!server_name_clean
    ServerAdmin webmaster@!server_name_clean
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/!server_name_clean_error.log
    CustomLog /var/log/apache2/!server_name_clean_access.log combined
    ServerSignature On

    Alias /server-status !document_root
    <Directory !document_root/serverstatus>
        SetHandler server-status
        Order deny,allow
        Deny from all
        Allow from 127.0.0.1
     </Directory>

    # ticket
    Alias /ticket !mount_point/links
    <Directory !mount_point/links>
      Options FollowSymLinks
      AllowOverride All
      Order deny,allow
      Allow from All
    </Directory>
</VirtualHost>

<VirtualHost *:80>
    ServerName app1.!server_name_clean
    ServerAdmin webmaster@!server_name_clean
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/app1.!server_name_clean_error.log
    CustomLog /var/log/apache2/app1.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName app2.!server_name_clean
    ServerAdmin webmaster@!server_name_clean
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/app2.!server_name_clean_error.log
    CustomLog /var/log/apache2/app2.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName upload.!server_name_clean
    ServerAdmin webmaster@!server_name_clean
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    <IfModule mod_php5.c>
        php_admin_value post_max_size 2008M
        php_admin_value upload_max_filesize 2000M
        php_admin_value memory_limit 128M
    </IfModule>

    ErrorLog /var/log/apache2/upload.!server_name_clean_error.log
    CustomLog /var/log/apache2/upload.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName download.!server_name_clean
    ServerAdmin webmaster@!server_name_clean
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/download.!server_name_clean_error.log
    CustomLog /var/log/apache2/download.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName job1.!server_name_clean
    ServerAdmin webmaster@!server_name_clean
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/job1.!server_name_clean_error.log
    CustomLog /var/log/apache2/job1.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName job2.!server_name_clean
    ServerAdmin webmaster@!server_name_clean
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/job2.!server_name_clean_error.log
    CustomLog /var/log/apache2/job2.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>") . '
</pre><p><li>Enable the website:</p><p><code>sudo a2ensite <b>your-site</b></code></p>
<p><li>Restart Apache:</p><p><code>sudo /etc/init.d/apache2 restart</code></p>',
    array(
      '!server_name_clean' => $server_name_clean,
      '!document_root' => $document_root,
      '!mount_point' => $mount_point,
    )
  );

  $form['apache'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Apache HTTP server setup'),
    '#description' => t("Choose a server setup. We recommend the Multiserver setup for production websites. The simple setup should only be used for testing purposes."),
  );

  $form['apache']['localhost'] = array(
    '#type' => 'radios',
    '#options' => array(
      'simple' => '<b>' . t("Simple setup, I'm testing") . '</b>',
      'advanced' => '<b>' . t("Multi server setup, I'm setting up production or development") . '</b>',
    ),
  );

  $form['apache']['local'] = array(
    '#type' => 'fieldset',
    '#title' => t('Localhost setup (testing)'),
    '#states' => array(
      'visible' => array(   // action to take.
        ':input[name="localhost"]' => array('value' => 'simple'),
      ),
    ),
  );
  $form['apache']['local']['local_text'] = array(
    '#markup' => $apache_settings_local,
  );

  $form['apache']['multi'] = array(
    '#type' => 'fieldset',
    '#title' => t('Multiserver setup (production / development)'),
    '#states' => array(
      'visible' => array(   // action to take.
        ':input[name="localhost"]' => array('value' => 'advanced'),
      ),
    ),
  );
  $form['apache']['multi']['multi_text'] = array(
    '#markup' => $apache_settings_adv,
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  return $form;
}

function mediamosa_profile_apache_settings_form_validate($form, &$form_state) {
  if (!in_array($form_state['values']['localhost'], array('simple', 'advanced'))) {
    form_set_error('', t('You must choose an setup.'));
  }
}

function mediamosa_profile_apache_settings_form_submit($form, &$form_state) {
  $server_name = _mediamosa_profile_server_name();
  variable_set('apache_setting', ($form_state['values']['localhost'] == 'simple' ? 'simple' : 'advanced'));

  if (variable_get('apache_setting') == 'simple') {
    db_update('mediamosa_server')
      ->fields(
        array(
          'server_uri' => 'http://' . $server_name . '/ticket/{TICKET}'
        ))
      ->condition('server_type', 'streaming', '=')
      ->execute();
    db_update('mediamosa_server')
      ->fields(
        array(
          'server_uri' => 'http://' . $server_name . '/download/{TICKET}'
        ))
      ->condition('server_type', 'download', '=')
      ->execute();
    db_update('mediamosa_server')
      ->fields(
        array(
          'server_uri' => 'http://' . $server_name . '/still/{TICKET}'
        ))
      ->condition('server_type', 'still', '=')
      ->execute();
    db_update('mediamosa_server')
      ->fields(
        array(
          'server_status' => 'OFF'
        ))
      ->condition('server_uri', 'http://job2.mediamosa.local', '=')
      ->execute();
    db_update('mediamosa_server')
      ->fields(
        array(
          'server_uri' => 'http://' . $server_name
        ))
      ->condition('server_uri', 'http://job1.mediamosa.local', '=')
      ->execute();
    variable_set('mediamosa_jobscheduler_uri', 'http://' . $server_name);
    db_update('mediamosa_server')
      ->fields(
        array(
          'server_uri' => 'http://' . $server_name . '/mediafile/upload?upload_ticket={TICKET}',
          'uri_upload_progress' => 'http://' . $server_name . '/mediafile/uploadprogress'
        ))
      ->condition('server_type', 'upload', '=')
      ->execute();
    variable_set('mediamosa_cron_url_app', 'http://' . $server_name);
  }
  else {
    variable_set('mediamosa_jobscheduler_uri', 'http://job1.' . $server_name);
    variable_set('mediamosa_cron_url_app', 'http://app1.' . $server_name);
  }

  if (strcasecmp($server_name, 'mediamosa.local')) {
    // We need to patch server table then.
    $results = db_select('mediamosa_server', 'ms')
      ->fields('ms')
      ->execute();

    // Lets save all entries in an array.
    $rows = array();
    foreach ($results as $row) {
      $rows[] = $row;
    }

    foreach ($rows as $row) {
      $server_uri = $revision_data['uri_upload_progress'] = str_replace('mediamosa.local', $server_name, $row->server_uri);
      $uri_upload_progress = $revision_data['uri_upload_progress'] = str_replace('mediamosa.local', $server_name, $row->uri_upload_progress);
      db_update('mediamosa_server')
        ->fields(
            array(
              'server_uri' => $server_uri,
              'uri_upload_progress' => $uri_upload_progress,
            ))
        ->condition('nid', $row->nid, '=')
        ->execute();
    }
  }
}

/**
 * Information about 1.7 -> 3.x migration.
 * Task callback.
 */
function mediamosa_profile_migration_form() {
  $form = array();

  // Add our css.
  drupal_add_css('profiles/mediamosa_profile/mediamosa_profile.css');

  // Get the server name.
  $server_name = _mediamosa_profile_server_name();

  // Migration.

  $form['migration'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Migrating your 1.7.x database to 3.x'),
    '#description' => t("If you already have an MediaMosa 1.x database, then you need to migrate the database to the new 3.x database format. Migrate 1.7.x database from your current 1.7.x MediaMosa installation to 2.x database by following these steps:
    <ol>
      <li>Open the <code>/default/settings.php</code> in your new MediaMosa 3.x installation in the <code>sites</code> directory.</li>
      <li>Insert the content below from the text box and change the settings to match your 1.7.x MySQL setup for the MediaMosa 1.x MySQL user. In the file there is already an commented out version you can edit.</li>
    </ol><p>You can start the migration process once you have completed the installation. To start the migration, go to MediaMosa home, then click on tab 'Configuration'. Click on the link 'MediaMosa 1.7.x migration tool' to open up the migration tool. The migration tool will pre-check before you can start the migration.</p>
    <p><b>Important notes:</b></p>
    <ul>
      <li>Both databases (1.7.x and 3.x) must be on the same MySQL database server. You can not migrate with 2 servers.</li>
      <li>Before you migrate, at least do the migration once for testing before planning the final migration to be sure the migration will be successful.</li>
      <li>All data will be migrated, except for the ticket and jobs tables. The ticket table holds current session for downloading and play tickets of mediafiles. The job tables hold information about running jobs and old jobs. So make sure that your server is no longer running any jobs when you migrate.</li>
      <li>The 1.7.x database will only be used for reading, nothing will change on your 1.7.x database. However, the 3.x database needs to be clean install for migration to be successful.</li>
    </ul>"),
  );

  $form['migration']['settings'] = array(
    '#markup' => '<p><b>' . t('Migration setup for sites/default/settings.php') . '</b><pre>' . htmlentities("\$databases['mig_memo']['default'] = array(
  'driver' => 'mysql',
  'database' => 'your_old_database',
  'username' => 'your_user_name',
  'password' => 'your_password',
  'host' => 'localhost'
);
\$databases['mig_memo_data']['default'] = array(
  'driver' => 'mysql',
  'database' => 'your_old_database_data',
  'username' => 'your_user_name',
  'password' => 'your_password',
  'host' => 'localhost'
);") . '</pre></p>',
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  return $form;
}

function mediamosa_profile_domain_usage_form() {
  $form = array();

  // Add our css.
  drupal_add_css('profiles/mediamosa_profile/mediamosa_profile.css');

  // Get the server name.
  $server_name = _mediamosa_profile_server_name();

  $form['domain'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t("Your domain, MediaMosa and Drupal's multiple sites"),
  );

  $form['domain']['apache_options'] = array(
    '#markup' => st("MediaMosa is setup by default using the 'mediamosa.local' DNS name. All REST interfaces, download and upload URLs use subdomains in our example setup. We use these subdomains as example; <ul>
    <li>http://mediamosa.local/ or http://admin.mediamosa.local/ as administration front-end.</li>
    <li>http://app1.mediamosa.local/ is an application REST interface.</li>
    <li>http://app2.mediamosa.local/ is an application REST interface.</li>
    <li>http://job1.mediamosa.local/ is an job REST interface used for transcoding and other job handeling tasks.</li>
    <li>http://job2.mediamosa.local/ is an job REST interface used for transcoding and other job handeling tasks.</li>
    <li>http://download.mediamosa.local/ is used for downloading files from MediaMosa.</li>
    <li>http://upload.mediamosa.local/ is used for uploading files to MediaMosa.</li>
    </ul>
    In the directory /sites in your MediaMosa installation, each of these DNS names do also exists as an directory, in each an example default.settings.php.<br />
    <br />
    It's important to know when using multiple subdomains for MediaMosa interfaces that each need an unique settings.php where at the end of the file an indentifier is used to indentify the interface. See our example default.settings.php files in each directory and notice the '\$conf['mediamosa_installation_id']' at the end of each (default.)settings.php file.<br />
    <br />
    Using multiple subdomains allows you to scale your MediaMosa installation to use more APP or more JOB servers.<br />
    <br />
    For more information how to setup our multiple subdomains read the !link on the Drupal website.
    ", array('!link' => l('Advanced and multisite installation', 'http://drupal.org/node/346385', array('attributes' => array('target' => '_blank'), 'absolute' => TRUE, 'external' => TRUE)))),
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  return $form;
}


/**
 * Advanced mkdir().
 * Check if the directory is exist, before makes it.
 * @param string $check_dir Directory to check.
 */
function _mediamosa_profile_mkdir($mountpoint, $check_dir) {
  if (!file_exists($mountpoint . $check_dir)) {
    mkdir($mountpoint . $check_dir);
  }

  // Make sure its a directory.
  assert(is_dir($mountpoint . $check_dir));

  if (!file_exists($mountpoint . '/data/simpletest')) {
    mkdir($mountpoint . '/data/simpletest');
  }

  // Simpletest version.
  if (!file_exists($mountpoint . '/data/simpletest' . $check_dir)) {
    mkdir($mountpoint . '/data/simpletest' . $check_dir);
  }
}

/**
 * Give back the server name.
 */
function _mediamosa_profile_server_name() {
  $server_name = url('', array('absolute' => TRUE));
  $server_name = drupal_substr($server_name, 0, -1);
  $server_name = drupal_substr($server_name, drupal_strlen('http://'));
  $server_name = check_plain($server_name);
  return $server_name;
}

/**
 * Give back the document root for install.php.
 */
function _mediamosa_profile_document_root() {
  // Document root.
  $script_filename = getenv('PATH_TRANSLATED');
  if (empty($script_filename)) {
    $script_filename = getenv('SCRIPT_FILENAME');
  }
  $script_filename = str_replace('', '/', $script_filename);
  $script_filename = str_replace('//', '/', $script_filename);
  $document_root = dirname($script_filename);
  return $document_root;
}
