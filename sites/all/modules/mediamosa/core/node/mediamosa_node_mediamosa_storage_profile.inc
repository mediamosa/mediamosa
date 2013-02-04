<?php
/**
 * @file
 * hook_load / -update / -delete / -insert hooks for node type
 * mediamosa_storage_profile.
 */

/**
 * Implements hook_load().
 */
function mediamosa_storage_profile_load(&$nodes) {
  $nodes = mediamosa_node_revision::load($nodes);

  // Load type specific info
  foreach ($nodes as $key => $node) {
    $nodes[$key] = module_invoke('mediamosa_storage_' . $node->{mediamosa_storage_profile_db::TYPE}, 'mediamosa_storage_profile_load', $node);
  }

  return $nodes;
}

/**
 * Implements hook_update().
 */
function mediamosa_storage_profile_update($node) {
  if ($node->revision) {
    mediamosa_storage_profile_insert($node);
  }
  else {
    // Get the field names of the table.
    $names = mediamosa_storage_profile_db::get_fields();

    // Exception is title.
    $node->{mediamosa_storage_profile_db::PROFILE} = $node->title;

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_storage_profile_db::TABLE_NAME, $names);
  }

  // Update type specific info
  module_invoke('mediamosa_storage_' . $node->{mediamosa_storage_profile_db::TYPE}, 'mediamosa_storage_profile_update', $node);
}

/**
 * Implements hook_insert().
 */
function mediamosa_storage_profile_insert($node) {
  // If just inserting another revision, don't trigger hook_mediamosa_storage_profile_insert
  $trigger_hook_mediamosa_storage_profile_insert = empty($node->{mediamosa_storage_profile_db::ID});

  // Get the field names of the table.
  $names = mediamosa_storage_profile_db::get_fields();

  // Exception is title.
  $node->{mediamosa_storage_profile_db::PROFILE} = $node->title;

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_storage_profile_db::TABLE_NAME, $names);

  // Insert revision does not return the storage_profile_id
  $result = db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
    ->fields('sp', array(mediamosa_storage_profile_db::ID))
    ->condition(mediamosa_storage_profile_db::NID, $node->{mediamosa_storage_profile_db::NID})
    ->condition(mediamosa_storage_profile_db::VID, $node->{mediamosa_storage_profile_db::VID})
    ->execute()
    ->fetchField();

  $node->{mediamosa_storage_profile_db::ID} = $result;

  // Insert type specific info
  if ($trigger_hook_mediamosa_storage_profile_insert) {
    module_invoke('mediamosa_storage_' . $node->{mediamosa_storage_profile_db::TYPE}, 'mediamosa_storage_profile_insert', $node);
  }
}

/**
 * Implements hook_delete().
 */
function mediamosa_storage_profile_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete_ex($node, mediamosa_storage_profile_db::TABLE_NAME);

  // Delete type specific info
  module_invoke('mediamosa_storage_' . $node->{mediamosa_storage_profile_db::TYPE}, 'mediamosa_storage_profile_delete', $node);
}

/**
 * Implements hook_node_revision_delete().
 */
function mediamosa_storage_profile_node_revision_delete($node) {
  mediamosa_node_revision::delete_revision($node);
}

/**
 * Implements hook_view().
 */
function mediamosa_storage_profile_view($node, $view_mode = 'full') {
  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Storage profiles'), 'admin/mediamosa/config/storage_profile');
  drupal_set_breadcrumb($breadcrumb);

  // Get the apps.
  $apps = mediamosa_app::collect_allowed_apps();
  $apps[0] = t('[global profile]');

  // Get list of all storage types
  $types = module_invoke_all('mediamosa_storage_info');

  // Default parameters.
  $rows = array(
    array(t('Title'), check_plain($node->title)),
    array(t('Storage profile id'), check_plain($node->storage_profile_id)),
    array(t('Application'), isset($apps[$node->app_id]) ? $apps[$node->app_id] : t('Unknown or deleted application')),
    array(t('Type'), $types[$node->{mediamosa_storage_profile_db::TYPE}]['name']),
  );

  // Storage type specific parameters
  $rows = array_merge($rows, module_invoke('mediamosa_storage_' . $node->{mediamosa_storage_profile_db::TYPE}, 'mediamosa_storage_profile_view', $node, $view_mode));

  // Add timestamps.
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['storage_profile'] = array(
    '#markup' => theme('table', array('header' => array(t('Parameter'), t('Value')), 'rows' => $rows)),
  );

  return $node;
}

/**
 * Implements hook_form().
 */
function mediamosa_storage_profile_form($node, $form_state) {
  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Storage profiles'), 'admin/mediamosa/config/storage_profile');
  if (isset($node->nid)) {
    $breadcrumb[] = l($node->title, 'admin/mediamosa/config/storage_profile/' . $node->nid);
  }
  drupal_set_breadcrumb($breadcrumb);

  // Get the apps.
  $apps = mediamosa_app::collect_allowed_apps();
  $apps[0] = t('[global profile]');

  // Limit the list of allowed apps
  if (count($apps)) {
    if (!user_access(mediamosa_permission::ADMIN_MEDIAMOSA . '|storage|' . $node->{mediamosa_storage_profile_db::TYPE})) {
      $allowed = false;
      foreach ($apps as $key => $app) {
        if (!user_access(mediamosa_permission::ADMIN_MEDIAMOSA . '|storage|' . $node->{mediamosa_storage_profile_db::TYPE} . '|app|' . $app[mediamosa_app_db::APP_ID])) {
          unset($apps[$key]);
        }
      }
    }
  }

  // If we're inserting a new node, set some defaults:
  if (!isset($node->nid)) {
    $node->app_id = 0;
    $node->file_extension = '';

    // Only administrators may add global profiles.
    if (!user_access(mediamosa_permission::ADMIN_MEDIAMOSA)) {
      unset($apps[0]);
    }
  }
  else {
    // Present a warning about changing storage profile parameters
    drupal_set_message(t('Changing storage parameters could break referential integrity between MediaMosa and your storage location. Only change them if necessary!'), 'warning');
  }

  $form = array();

  $form['required'] = array(
    '#type' => 'fieldset',
    '#title' => t('Storage profile'),
    '#description' => t('Required parameters for a storage profile.'),
  );

  if (isset($node->storage_profile_id)) {
    $form['required']['storage_profile_id_markup'] = array(
      '#title' => t('Storage profile id'),
      '#type' => 'item',
      '#markup' => $node->storage_profile_id,
    );
    $form['required']['storage_profile_id'] = array(
      '#type' => 'hidden',
      '#value' => $node->storage_profile_id,
    );
  }

  $form['required'][mediamosa_storage_profile_db::TYPE] = array(
    '#type' => 'hidden',
    '#value' => $node->{mediamosa_storage_profile_db::TYPE},
  );

  $form['required'][mediamosa_storage_profile_db::APP_ID] = array(
    '#title' => t('Application'),
    '#type' => 'select',
    '#default_value' => $node->{mediamosa_storage_profile_db::APP_ID},
    '#required' => TRUE,
    '#options' => $apps,
    '#description' => t('The application that owns this profile.'),
  );

  $form['required']['title'] = array(
    '#title' => t('Profile name'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#description' => t('The name of this storage profile.'),
    '#default_value' => mediamosa_node::default_value('title', $node),
    '#maxlength' => mediamosa_storage_profile_db::PROFILE_LENGTH,
  );

  // Storage type specific parameters
  $types = module_invoke_all('mediamosa_storage_info');
  $form[$node->{mediamosa_storage_profile_db::TYPE}] = array(
      '#type' => 'fieldset',
      '#title' => t($types[$node->{mediamosa_storage_profile_db::TYPE}]['name']),
      '#description' => t('Parameters for ' . $types[$node->{mediamosa_storage_profile_db::TYPE}]['name'] . '.'),
  );

  if (module_hook('mediamosa_storage_' . $node->{mediamosa_storage_profile_db::TYPE}, 'mediamosa_storage_profile_form')) {
    $form[$node->{mediamosa_storage_profile_db::TYPE}] += module_invoke('mediamosa_storage_' . $node->{mediamosa_storage_profile_db::TYPE}, 'mediamosa_storage_profile_form', $node, $form_state);
  }
  else {
    $form[$node->{mediamosa_storage_profile_db::TYPE}]['no_parameters'] = array(
      '#type' => 'item',
      '#markup' => t('No parameters'),
    );
  }

  return $form;
}

/**
 * Implements hook_node_validate().
 */
function mediamosa_storage_node_validate($node, $form, &$form_state) {
  if (!empty($node->{mediamosa_storage_profile_db::TYPE})) {
    module_invoke('mediamosa_storage_' . $node->{mediamosa_storage_profile_db::TYPE}, 'mediamosa_storage_profile_validate', $node, $form, $form_state);
  }
}

/**
 * Implements hook_node_presave().
 */
function mediamosa_storage_node_presave($node) {
  if (!empty($node->{mediamosa_storage_profile_db::TYPE})) {
    module_invoke('mediamosa_storage_' . $node->{mediamosa_storage_profile_db::TYPE}, 'mediamosa_storage_profile_presave', $node);
  }
}
