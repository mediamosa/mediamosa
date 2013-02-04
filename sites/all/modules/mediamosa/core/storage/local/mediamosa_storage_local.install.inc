<?php
/**
 * @file
 * Install, update, and uninstall functions for the mediamosa local storage module.
 */

/**
 * Implements hook_install().
 */
function _mediamosa_storage_local_install() {
  // Create the one and only default local storage.
  $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE, 'MediaMosa local storage');
  $node->{mediamosa_storage_profile_db::TYPE} = mediamosa_storage_local::NAME;
  $node->{mediamosa_storage_profile_db::APP_ID} = 0;
  $node->{mediamosa_storage_profile_db::VERSION} = 0;
  $node->{mediamosa_storage_profile_db::PROFILE} = 'MediaMosa local storage';
  node_save($node);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_storage_local_uninstall() {
  // Delete all nodes for this storage type
  $result = db_select(mediamosa_storage_profile_db::TABLE_NAME, 'p')
    ->condition(mediamosa_storage_profile_db::TYPE, mediamosa_storage_local::NAME)
    ->fields('p', array(mediamosa_storage_profile_db::NID))
    ->execute();

  foreach ($result as $record) {
    node_delete($record->{mediamosa_storage_profile_db::NID});
  }

  // Remove our variables.
  $result = db_select('variable', 'v')
    ->fields('v', array('name'))
    ->condition('name', db_like('mediamosa_storage_local_') . '%', 'LIKE')
    ->execute();

  foreach ($result as $row) {
    variable_del($row->name);
  }
}
