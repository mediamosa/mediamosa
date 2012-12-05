<?php
/**
 * @file
 * Install, update, and uninstall functions for the mediamosa standard storage module.
 */

/**
 * Implements hook_install().
 */
function _mediamosa_storage_standard_install() {
  // Create the one and only default standard storage
  $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE, 'Standard storage');
  $node->{mediamosa_storage_profile_db::TYPE} = mediamosa_storage_standard::NAME;
  $node->{mediamosa_storage_profile_db::APP_ID} = 0;
  $node->{mediamosa_storage_profile_db::VERSION} = 0;
  $node->{mediamosa_storage_profile_db::PROFILE} = 'Standard storage';
  $node->{mediamosa_storage_profile_db::IS_DEFAULT_PROFILE} = mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_TRUE;
  node_save($node);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_storage_standard_uninstall() {
  // Delete all nodes for this storage type
  $result = db_select(mediamosa_storage_profile_db::TABLE_NAME, 'p')
    ->condition(mediamosa_storage_profile_db::TYPE, mediamosa_storage_standard::NAME)
    ->fields('p', array(mediamosa_storage_profile_db::NID))
    ->execute();

  foreach ($result as $record) {
    node_delete($record->{mediamosa_storage_profile_db::NID});
  }  
  
  // Remove our variables.
  $result = db_select('variable', 'v')
    ->fields('v', array('name'))
    ->condition('name', db_like('mediamosa_storage_standard_') . '%', 'LIKE')
    ->execute();

  foreach ($result as $row) {
    variable_del($row->name);
  }
}
