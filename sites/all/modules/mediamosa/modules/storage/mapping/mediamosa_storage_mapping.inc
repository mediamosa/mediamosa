<?php
// $Id$

 /**
  * @file
  * MediaMosa storage mapping.
  */

/**
 * Show the listing of the current storage mappings for the given application
 * 
 * @paran $app
 *   Application node
 *
 * @see _mediamosa_storage_profile_list
 */
function _mediamosa_storage_mapping_app_list($app) {
  $app_id = $app->{mediamosa_app_db::APP_ID};
  $header = array(
    array('data' => t('Profile'), 'field' => 'sp.' . mediamosa_storage_profile_db::PROFILE),
    array('data' => t('Type'), 'field' => 'sp.' . mediamosa_storage_profile_db::TYPE),
    array('data' => t('Path'), 'field' => 'sm.' . mediamosa_storage_mapping_db::PATH),
    array('data' => t('Is Default'), 'field' => 'sm.' . mediamosa_storage_mapping_db::IS_DEFAULT_PROFILE),
    t('Operations')
  );

  $result = mediamosa_db::db_select(mediamosa_storage_mapping_db::TABLE_NAME, 'sm')
    ->addJoin('INNER', mediamosa_storage_profile_db::TABLE_NAME, 'sp', 'sm.%profile_id = sp.%id', array('%profile_id' => mediamosa_storage_mapping_db::PROFILE_ID, '%id' => mediamosa_storage_profile_db::ID))
    ->condition(mediamosa_storage_mapping_db::APP_ID, $app_id)
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->fields('sm')
    ->fields('sp')
    ->orderByHeader($header)
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->execute();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());
  
  // Get storage types
  $types = module_invoke_all('mediamosa_storage_info');

  $rows = array();
  foreach ($result as $mapping) {
    $actions = array();
    if ($mapping[mediamosa_storage_mapping_db::IS_DEFAULT_PROFILE] == mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_FALSE) {
      //TODO: actions not implemented yet since only default mappings are supported
      if (node_access('update', $app)) {
        $actions[] = l(t('Remove'), 'admin/mediamosa/config/app/' . $app->{mediamosa_app_db::NID} . '/storage_mapping/' . $mapping[mediamosa_storage_mapping_db::ID] . '/delete', $l_options);
      }
    }

    $rows[] = array(
      'profile' => array('data' => l($mapping[mediamosa_storage_profile_db::PROFILE], 'admin/mediamosa/config/storage_profile/' . $mapping[mediamosa_storage_profile_db::NID]), 'field' => $mapping[mediamosa_storage_profile_db::PROFILE]),
      'type' => $types[$storage_profile[mediamosa_storage_mapping_db::TYPE]]['name'],
    	'path' => $mapping[mediamosa_storage_mapping_db::PATH],
    	'default' => $mapping[mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING],
      empty($actions) ? '-' : implode(' | ', $actions),
    );
  }

  $output = array();
  $output[] = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('There are no storage mappings for this client application.')));
  $output[] = theme('pager');
  return implode('', $output);
}

/**
* Show the listing of the current storage mappings for the given profile
* 
* @param $profile
*   Storage profile node
*
* @see mediamosa_storage_profile_view
*/
function _mediamosa_storage_mapping_profile_list($profile) {
  $profile_id = $profile->{mediamosa_storage_profile_db::ID};
  $header = array(
    array('data' => t('Application'), 'field' => 'app.' . mediamosa_app_db::APP_NAME),
    array('data' => t('Path'), 'field' => 'sm.' . mediamosa_storage_mapping_db::PATH),
    array('data' => t('Is Default'), 'field' => 'sm.' . mediamosa_storage_mapping_db::IS_DEFAULT_PROFILE),
    t('Operations')
  );

  $result = mediamosa_db::db_select(mediamosa_storage_mapping_db::TABLE_NAME, 'sm')
    ->addJoin('INNER', mediamosa_app_db::TABLE_NAME, 'app', 'sm.%app_id = app.%id', array('%app_id' => mediamosa_storage_mapping_db::APP_ID, '%id' => mediamosa_app_db::APP_ID))
    ->condition(mediamosa_storage_mapping_db::PROFILE_ID, $profile_id)
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->fields('sm')
    ->fields('app')
    ->orderByHeader($header)
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->execute();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  $rows = array();
  foreach ($result as $mapping) {
    $actions = array();
    if ($mapping[mediamosa_storage_mapping_db::IS_DEFAULT_PROFILE] == mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_FALSE) {
      //TODO: actions not implemented yet since only default mappings are supported
      if (node_access('update', node_load($mapping[mediamosa_app_db::NID], $mapping[mediamosa_app_db::VID]))) {
        $actions[] = l(t('Remove'), 'admin/mediamosa/config/app/' . $mapping[mediamosa_app_db::NID] . '/storage_mapping/' . $mapping[mediamosa_storage_mapping_db::ID] . '/delete', $l_options);
      }
    }

    $rows[] = array(
      'application' => array('data' => l($mapping[mediamosa_app_db::APP_NAME], 'admin/mediamosa/config/app/' . $mapping[mediamosa_app_db::NID]), 'field' => $mapping[mediamosa_app_db::APP_NAME]),
    	'path' => $mapping[mediamosa_storage_mapping_db::PATH],
    	'default' => $mapping[mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING],
      empty($actions) ? '-' : implode(' | ', $actions),
    );
  }

  $output = array();
  $output[] = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('There are no storage mappings to this storage profile.')));
  $output[] = theme('pager');
  return implode('', $output);
}
