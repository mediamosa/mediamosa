<?php
/**
 * @file
 * MediaMosa installation profile.
 */

// Include our helper class as autoloader is not up yet.
require_once('mediamosa_profile.class.inc');

/**
 * Implements hook_install_tasks().
 */
function mediamosa_profile_install_tasks() {

  // Add our css.
  drupal_add_css('profiles/mediamosa_profile/mediamosa_profile.css');

  // Set our title.
  drupal_set_title(mediamosa_profile::get_title());

  // Setup the tasks.
  return array(
    'mediamosa_profile_metadata_support_form' => array(
      'display_name' => st('Metadata support'),
      'type' => 'form',
    ),
    'mediamosa_profile_storage_location_form' => array(
      'display_name' => st('Storage location'),
      'type' => 'form',
      'run' => variable_get('mediamosa_current_mount_point', '') ? INSTALL_TASK_SKIP : INSTALL_TASK_RUN_IF_NOT_COMPLETED,
    ),
    'mediamosa_profile_apache_settings_form' => array(
      'display_name' => st('Apache settings'),
      'type' => 'form',
    ),
    'mediamosa_profile_domain_usage_form' => array(
      'display_name' => st('Your domain usage'),
      'type' => 'form',
      'run' => variable_get('mediamosa_apache_setting') == 'simple' ? INSTALL_TASK_SKIP : INSTALL_TASK_RUN_IF_NOT_COMPLETED,
    ),
    'mediamosa_profile_cron_settings_form' => array(
      'display_name' => st('Cron settings'),
      'type' => 'form',
    ),
  );
}

/**
 * Implements hook_install_tasks_alter().
 */
function mediamosa_profile_install_tasks_alter(&$tasks, $install_state) {

  // Our requirements step.
  $mediamosa_profile_php_settings_form = array(
    'display_name' => st('MediaMosa requirements'),
    'type' => 'form',
  );

  // We need to rebuild tasks in the same order and put our
  // 'mediamosa_profile_php_settings_form' between it.
  $tasks_rebuild = array();
  foreach ($tasks as $name => $task) {
    // Copy task.
    $tasks_rebuild[$name] = $task;

    // If we reach certain point, then insert our task.
    if ($name == 'install_bootstrap_full') {
      $tasks_rebuild['mediamosa_profile_php_settings_form'] = $mediamosa_profile_php_settings_form;
    }
  }

  // Copy rebuild.
  $tasks = $tasks_rebuild;
}

/**
 * Get the mount point.
 * Task callback.
 */
function mediamosa_profile_metadata_support_form() {
  $form = array();

  $options = array(
    'dublin_core' => st('Dublin Core'),
    'qualified_dublin_core' => st('Qualified Dublin Core'),
    'czp' => st('Content Zoek Profiel (Content Search Profile)'),
  );

  $form['description'] = array(
    '#markup' => '<p><b>' . st('Select the types of Metadata you want to support in your MediaMosa installation. Any of these metadata libraries can be enabled or disabled later by enabling or disabling the metadata module of its type.') . '</b></p>',
  );

  $form['metadata_support'] = array(
    '#type' => 'checkboxes',
    '#title' => st('Select the metadata libraries you want to use.'),
    '#description' => st('For more information about Dublin Core !link_dc. For more information about Qualified Dublin Core !link_qdc. For more information about Content Zoek Profiel !link_czp (Dutch)', array('!link_dc' => l('click here', 'http://dublincore.org', array('absolute' => TRUE, 'attributes' => array('target' => '_blank'))), '!link_qdc' => l('click here', 'http://dublincore.org/documents/usageguide/qualifiers.shtml', array('absolute' => TRUE, 'attributes' => array('target' => '_blank'))), '!link_czp' => l('click here', 'http://www.edustandaard.nl/afspraken/001', array('absolute' => TRUE, 'attributes' => array('target' => '_blank'))))),
    '#options' => $options,
    '#required' => TRUE,
    '#default_value' => array('dublin_core', 'qualified_dublin_core'),
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => st('Continue'),
  );

  return $form;
}

/**
 * Process the submit of the metadata selection form.
 */
function mediamosa_profile_metadata_support_form_submit(&$form, &$form_state) {
  // Get the values from the submit.
  $values = $form_state['values'];

  // List of our metadata modules.
  $to_enable = array(
    'dublin_core' => 'mediamosa_metadata_dc',
    'qualified_dublin_core' => 'mediamosa_metadata_qdc',
    'asset' => 'mediamosa_metadata_asset',
    'czp' => 'mediamosa_metadata_czp',
  );

  // Enable the metadata modules that where selected.
  foreach ($to_enable as $type => $module) {
    if (!empty($values['metadata_support'][$type]) && $values['metadata_support'][$type] == $type) {
      module_enable(array($module));
    }
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function system_form_install_settings_form_alter(&$form, $form_state, $form_id) {
  // Set default for site name field.
  $form['intro'] = array(
    '#weight' => -1,
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => st('Setting up database'),
    '#description' => st("
   <p>We advice using !mysql v5.1, or use MySQL variant like !mariadb.</p>
   <p>This version of MediaMosa will also work with !postgresql.</p>
   <p>Use the database <b>mediamosa</b> example below to create your database 'mediamosa' with user 'mediamosa' before proceeding.</p>
    <pre>
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
    </pre>
    <p>
        You may change the 'mediamosa' database prefix and the database user name.<br />
        <br />
        If you want to migrate your current MediaMosa v1.7 database to the new 3.x version, you have to create or have a database user, which has enough rights to read your current v1.7 databases.</p>
   ", array(
    '!mysql' => l('MySQL', 'http://mysql.com', array('absolute' => TRUE)),
    '!mariadb' => l('MariaDB', 'http://mariadb.org', array('absolute' => TRUE)),
    '!postgresql' => l('PostgreSQL', 'http://www.postgresql.org', array('absolute' => TRUE))
   ))
  );
}

/**
 * Implements hook_form_alter().
 */
function system_form_install_configure_form_alter(&$form, &$form_state, $form_id) {
  $form['site_information']['site_name']['#default_value'] = 'MediaMosa';
  $form['site_information']['site_mail']['#default_value'] = 'webmaster@' . $_SERVER['SERVER_NAME'];
  $form['admin_account']['account']['name']['#default_value'] = 'admin';
  $form['admin_account']['account']['mail']['#default_value'] = 'admin@' . $_SERVER['SERVER_NAME'];
}

/**
 * Show a checklist of the installation.
 */
function mediamosa_profile_php_settings_form($form, &$form_state, &$install_state) {

  // Requirements for PHP modules.
  $php_modules = mediamosa_profile::requirements_php_modules();
  $form['requirements']['php_modules']['title'] = array(
    '#markup' => '<h1>' . st('PHP Modules') . '</h1>'
  );
  $form['requirements']['php_modules']['requirements'] = array(
    '#markup' => theme('status_report', array('requirements' => $php_modules['requirements']))
  );

  // Required installed 3rd party programs.
  $installed_programs = mediamosa_profile::requirements_installed_programs();
  $form['requirements']['installed_programs']['title'] = array(
    '#markup' => '<h1>' . st('Installed programs') . '</h1>'
  );
  $form['requirements']['installed_programs']['requirements'] = array(
    '#markup' => theme('status_report', array('requirements' => $installed_programs['requirements']))
  );

  // Required PHP settings.
  $php_settings = mediamosa_profile::requirements_php_settings();
  $form['requirements']['php_settings']['title'] = array(
    '#markup' => '<h1>' . st('PHP variables / Settings') . '</h1>'
  );
  $form['requirements']['php_settings']['requirements'] = array(
    '#markup' => theme('status_report', array('requirements' => $php_settings['requirements']))
  );

  // Check for errors.
  if ($php_modules['errors'] + $installed_programs['errors'] + $php_settings['errors']) {
    $form['requirements']['errors']['text'] = array(
      '#markup' => '<p><b>' . st("Solve the reported problems and press 'continue' to continue.") . '</b></p>',
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
  // Get the requirements.
  $php_modules = mediamosa_profile::requirements_php_modules();
  $installed_programs = mediamosa_profile::requirements_installed_programs();
  $php_settings = mediamosa_profile::requirements_php_settings();

  // Check for errors.
  if ($php_modules['errors'] + $installed_programs['errors'] + $php_settings['errors']) {
    form_set_error('foo', st('Solve the reported problems below before you continue. You can ignore the (yellow) warnings.'));
  }
}

/**
 * Get the mount point.
 * Task callback.
 */
function mediamosa_profile_storage_location_form() {
  $form = array();

  // Default mount pount.
  $mount_point = variable_get('mediamosa_current_mount_point', '/srv/mediamosa');

  $form['description'] = array(
    '#markup' => '<p><b>' . st('The MediaMosa mount point is a shared directory where related mediafiles, images and other files are stored. On a multi-server setup, this mount point needs to be available for all servers (i.e. through NFS)') . '</b></p>',
  );

  $form['current_mount_point'] = array(
    '#type' => 'textfield',
    '#title' => t('MediaMosa SAN/NAS Mount point'),
    '#description' => st('Make sure the Apache user has write access to the MediaMosa SAN/NAS mount point.'),
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
    form_set_error('current_mount_point', t('The current Linux mount point is not writeable for the webserver (@server_software).', array('@server_software' => $_SERVER['SERVER_SOFTWARE'])));
  }
}

function mediamosa_profile_storage_location_form_submit($form, &$form_state) {
  // Get the form values.
  $values = $form_state['values'];

  // Set our variables.
  variable_set('mediamosa_current_mount_point', $values['current_mount_point']);

  // Profile does not does not handle Windows installations.
  variable_set('mediamosa_current_mount_point_windows', '\\');

  // Inside the storage location, create a MediaMosa storage structure.
  mediamosa_profile::mountpoint_mkdir('data');
  mediamosa_profile::mountpoint_mkdir('data/stills');

  // We store each file in separate directories based on the first letter of the
  // file. We need to create these directories.
  $prefixes = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXZ';
  for ($x = 0; $x < strlen($prefixes); $x++) {
    mediamosa_profile::mountpoint_mkdir('data/' . $prefixes{$x});
    mediamosa_profile::mountpoint_mkdir('data/stills/' . $prefixes{$x});
  }

  // Other.
  mediamosa_profile::mountpoint_mkdir('data/transcode');
  mediamosa_profile::mountpoint_mkdir('links');
  mediamosa_profile::mountpoint_mkdir('download_links');
  mediamosa_profile::mountpoint_mkdir('ftp');

  // /media is replacing /still_links.
  mediamosa_profile::mountpoint_mkdir('media');
  mediamosa_profile::mountpoint_mkdir('media/ticket');

  // Create the htaccess file to protected the media directory.
  mediamosa_configuration_storage::file_create_htaccess($values['current_mount_point'] . '/media', mediamosa_configuration_storage::media_get_htaccess_contents());
}

/**
 * Information about cron, apache and migration.
 */
function mediamosa_profile_cron_settings_form() {
  $form = array();

  // Get the server name.
  $server_name = mediamosa_profile::get_server_name();
  if (variable_get('mediamosa_apache_setting') == 'simple') {
    $server_name = 'localhost';
  }

  // Cron.
  $form['cron'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => st('Cron setup'),
    '#description' => t('The cron will be used trigger MediaMosa every minute for background jobs. The setup for cron is required for MediaMosa to run properly.'),
  );

  $form['cron']['crontab_text'] = array(
    '#markup' => st('<h5>Add a crontab entry</h5>Modify your cron using crontab, this will run the script every minute:<p><code>crontab -e</code></p><p>Add this line at the bottom:</p>'),
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

  // Get the server name.
  $server_name = mediamosa_profile::get_server_name();
  $mount_point = variable_get('mediamosa_current_mount_point', '');

  $apache_settings_local = st("This setup is for installation of MediaMosa on 1 server and 1 domain.
  Can be used for simple testing setups, but also for a small server deploy. MediaMosa can be installed on a subdirectory
  (http://domain/mediamosa), or in the document root (http://domain).
  <p><li>Add the following lines to your default apache definition:</p>
    <pre>" . htmlentities("
    # MediaMosa tickets
    Alias !rel_directoryticket !mount_point/links
    <Directory !mount_point/links>
      Options FollowSymLinks
      Order deny,allow
      Allow from All
    </Directory>

    # Media
    Alias !rel_directorymedia !mount_point/media
    <Directory !mount_point/media>
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
      '!server_name_clean' => $server_name,
      '!document_root' => DRUPAL_ROOT,
      '!rel_directory' => url(),
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

    # Media
    Alias /media !mount_point/media
    <Directory !mount_point/media>
      Options FollowSymLinks
      AllowOverride All
      Order deny,allow
      Allow from All
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
      '!document_root' => DRUPAL_ROOT,
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
      'simple' => '<b>' . t("Single server / domain setup.") . '</b>',
      'advanced' => '<b>' . t("Multiple server / domain setup.") . '</b>',
    ),
  );

  $form['apache']['local'] = array(
    '#type' => 'fieldset',
    '#title' => t('Single server / domain setup.'),
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
    '#title' => t('Multi server/domain setup.'),
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
    form_set_error('', t('You must choose a setup.'));
  }
}

function mediamosa_profile_apache_settings_form_submit($form, &$form_state) {
  // Get current url.
  $server_name = mediamosa_profile::get_server_name();

  // Save the chosen install state.
  variable_set('mediamosa_apache_setting', ($form_state['values']['localhost'] == 'simple' ? 'simple' : 'advanced'));

  // Simple type of server? Then change default setup of advanced.
  if (variable_get('mediamosa_apache_setting') == 'simple') {
    // Simple URL for job and app.
    variable_set('mediamosa_jobscheduler_uri', 'http://' . $server_name . '/');
    variable_set('mediamosa_cron_url_app', 'http://' . $server_name . '/');

    // Get all mediamosa_server nodes nids, and update server_uri.
    $results = db_select('mediamosa_server', 'ms')
      ->fields('ms', array(mediamosa_server_db::NID))
      ->execute();

    // Walk through the server nodes and save the simple url.
    foreach ($results as $result) {
      $node = node_load($result->{mediamosa_server_db::NID});
      $node->{mediamosa_server_db::SERVER_URI} = 'http://' . $server_name . '/';
      node_save($node);
    }
  }
  else {
    // Advanced URLs.
    variable_set('mediamosa_jobscheduler_uri', 'http://job1.' . $server_name . '/');
    variable_set('mediamosa_cron_url_app', 'http://app1.' . $server_name . '/');
  }
}

function mediamosa_profile_domain_usage_form() {
  $form = array();

  $form['domain'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t("Your domain, MediaMosa and Drupal's multiple sites"),
  );

  $form['domain']['apache_options'] = array(
    '#markup' => st(
    "Your MediaMosa setup is using the '!host' DNS name. All REST interfaces, download and upload URLs use these subdomains in your current setup. We use these subdomains as example; <ul>
    <li>http://!host/ or http://admin.!host/ as administration front-end.</li>
    <li>http://app1.!host/ is an application REST interface.</li>
    <li>http://app2.!host/ is an application REST interface.</li>
    <li>http://job1.!host/ is an job REST interface used for transcoding and other job handeling tasks.</li>
    <li>http://job2.!host/ is an job REST interface used for transcoding and other job handeling tasks.</li>
    <li>http://download.!host/ is used for downloading files from MediaMosa.</li>
    <li>http://upload.!host/ is used for uploading files to MediaMosa.</li>
    </ul>
    In the directory /sites in your MediaMosa installation, each of these DNS names do also exists as an directory, in each an example default.settings.php.<br />
    <br />
    It's important to know when using multiple subdomains for MediaMosa interfaces that each need an unique settings.php where at the end of the file an indentifier is used to indentify the interface. See our example default.settings.php files in each directory and notice the '\$conf['mediamosa_installation_id']' at the end of each (default.)settings.php file.<br />
    <br />
    Using multiple subdomains allows you to scale your MediaMosa installation to use more APP or more JOB servers.<br />
    <br />
    For more information how to setup our multiple subdomains read the !link on the Drupal website.",
    array(
      '!host' => mediamosa_profile::get_host(),
      '!link' => l('Advanced and multisite installation', 'http://drupal.org/node/346385', array('attributes' => array('target' => '_blank'), 'absolute' => TRUE, 'external' => TRUE)),
    )),
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  return $form;
}
