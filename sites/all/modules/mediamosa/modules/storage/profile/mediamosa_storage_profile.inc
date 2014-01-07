<?php
/**
 * @file
 * MediaMosa storage profile function.
 */

/**
 * Show the listing of the current storage profiles.
 */
function _mediamosa_storage_profile_list() {

  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  drupal_set_breadcrumb($breadcrumb);

  // Get the apps.
  $apps = mediamosa_app::collect_allowed_apps();
  $apps[0] = t('[global profile]');

  // Only allow either all apps (admin) or limited to global and owned.
  $app_ids = array_keys($apps);

  $header = array(
    array('data' => t('Storage profile id'), 'field' => 'sp.' . mediamosa_storage_profile_db::ID),
    array('data' => t('Profile'), 'field' => 'sp.' . mediamosa_storage_profile_db::PROFILE),
    array('data' => t('Type'), 'field' => 'sp.' . mediamosa_storage_profile_db::TYPE),
    array('data' => t('App owner'), 'field' => 'sp.' . mediamosa_storage_profile_db::APP_ID),
    t('Operations')
  );

  $query = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')->extend('PagerDefault')->extend('TableSort');
  $query->fields('sp');

  // Limit apps?
  if (!empty($app_ids)) {
    $query->condition(mediamosa_storage_profile_db::APP_ID, $app_ids, 'IN');
  }

  $storage_profiles = $query->orderByHeader($header)
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->execute()->fetchAll();

  // Collect the node IDs.
  $nids = array();
  foreach ($storage_profiles as $storage_profile) {
    $nids[] = $storage_profile[mediamosa_storage_profile_db::NID];
  }

  // Get the nodes.
  $nodes = node_load_multiple($nids);

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  // Get storage types
  $types = module_invoke_all('mediamosa_storage_info');

  $rows = array();
  foreach ($storage_profiles as $storage_profile) {
    $actions = array();
    if (node_access('update', $nodes[$storage_profile[mediamosa_storage_profile_db::NID]])) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/config/storage_profile/' . $storage_profile[mediamosa_storage_profile_db::NID] . '/edit', $l_options);
    }
    if (node_access('delete', $nodes[$storage_profile[mediamosa_storage_profile_db::NID]])) {
      $actions[] = l(t('Delete'), 'admin/mediamosa/config/storage_profile/' . $storage_profile[mediamosa_storage_profile_db::NID] . '/delete', $l_options);
    }

    $owner = ($storage_profile[mediamosa_storage_profile_db::APP_ID] == 0) ? 'global profile' : $storage_profile[mediamosa_storage_profile_db::APP_ID];
    $rows[] = array(
      'storage_profile_id' => $storage_profile[mediamosa_storage_profile_db::ID],
      'title' => array('data' => l($storage_profile[mediamosa_storage_profile_db::PROFILE], 'admin/mediamosa/config/storage_profile/' . $storage_profile[mediamosa_storage_profile_db::NID]), 'field' => $storage_profile[mediamosa_storage_profile_db::PROFILE]),
      'type' => $types[$storage_profile[mediamosa_storage_profile_db::TYPE]]['name'],
      'owner' => $owner,
      empty($actions) ? '-' : implode(' | ', $actions),
    );
  }

  $output = array();
  $output[] = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('You have no storage profiles. Click on the <em>Add</em> tab to create one.')));
  $output[] = theme('pager');
  return implode('', $output);
}

/**
 * Add a new storage profile
 *
 * @param string $type
 * 	 Optional type of the new profile to create
 */
function _mediamosa_storage_profile_add_page($storage_type = NULL) {
  global $user;

  // Get allowed apps
  $apps = mediamosa_app::collect_allowed_apps();
  $apps[0] = t('[global profile]');

  // Build list of allowed storage types
  $storage_types = module_invoke_all('mediamosa_storage_info');

  if (count($storage_types) || $storage_types['standard']) {
    if (count($apps)) {
      if (class_exists('mediamosa_storage_local')) {
        // Never allow to add extra local storage.
        unset($storage_types[mediamosa_storage_local::NAME]);
      }
      foreach ($storage_types as $key => $type) {
        if (!user_access(mediamosa_permission::ADMIN_MEDIAMOSA . '|storage|' . $key)) {
          $allowed = FALSE;
          foreach ($apps as $app) {
            $allowed |= (user_access(mediamosa_permission::ADMIN_MEDIAMOSA . '|storage|' . $key . '|app|' . $app[mediamosa_app_db::APP_ID] . '|add'));
          }
          if (!$allowed) {
            unset($storage_types[$key]);
          }
        }
      }
    }

    if (empty($storage_types)) {
      return t('You are not allowed to add storage profiles for your client applications or you haven\'t enabled any other storage module(s) than local.');
    }
  }
  else {
    return t('No storage types found. Enable one or more storage modules.');
  }

  if ($type && array_key_exists($storage_type, $storage_types)) {
    $node_types = node_type_get_types();
    $node = (object) array('uid' => $user->uid, 'name' => (isset($user->name) ? $user->name : ''), 'type' => mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE, 'language' => LANGUAGE_NONE, mediamosa_storage_profile_db::TYPE => $storage_type);
    drupal_set_title(t('Create @name', array('@name' => $node_types[mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE]->name)), PASS_THROUGH);
    return drupal_get_form(mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE . '_node_form', $node);
  }
  elseif (count($storage_types) == 1) {
    $storage_type = array_shift(array_keys($storage_types));
    drupal_goto('admin/mediamosa/config/storage_profile/add/' . $storage_type);
  }

  $content = array();
  foreach ($storage_types as $key => $type) {
    $content[] = array(
      'title' => $type['name'],
      'href' => 'admin/mediamosa/config/storage_profile/add/'.$key,
      'description' => $type['description'],
    );
  }
  return theme('node_add_list', array('content' => $content));
}

/**
 * Confirm or prevent node deletion.
 */
function _mediamosa_storage_profile_delete_confirm($form, &$form_state, $node) {
  //TODO: prevent some profile from being deleted (like standard profile, profiles in use, ...)

  // Call the node delete confirm form.
  return _mediamosa_node_delete_confirm($form, $form_state, $node);
}

/**
* Execute node deletion
*/
function _mediamosa_storage_profile_delete_confirm_submit($form, &$form_state) {
  //TODO: execute some predelete actions (like moving files to another profile)

  // Call the node confirm submit function.
  _mediamosa_node_delete_confirm_submit($form, $form_state);
}

/**
 * Return storage options for given client app
 *
 * @param integer $app_id
 *   Client application ID
 *
 * @return array
 *   The options array for select items.
 */
function _mediamosa_storage_profile_app_options($app_id) {
  $options = array();
  $default_storage_option = FALSE;

  // Get the storage profiles.
  // @todo: access check?
  $storage_profiles = mediamosa_storage_profile::get_all($app_id, array(mediamosa_storage_profile_db::ID, mediamosa_storage_profile_db::PROFILE));

  foreach ($storage_profiles as $storage_profile) {
    foreach (mediamosa_storage_profile::get_storage_options($storage_profile[mediamosa_storage_profile_db::ID]) as $key => $label) {
      $options[$storage_profile[mediamosa_storage_profile_db::PROFILE]][$key] = $label;

      // Find the first default.
      if ($storage_profile[mediamosa_storage_profile_db::ID] == mediamosa_storage_profile::DEFAULT_STORAGE_PROFILE_ID) {
        $default_storage_option = $key;
      }
    }
  }

  // Add possible found default storage option.
  $options['#default_storage_option'] = $default_storage_option;

  return $options;
}
