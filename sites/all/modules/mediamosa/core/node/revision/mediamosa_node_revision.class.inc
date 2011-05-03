<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * Static functions for mediamosa_node_revisions.
  */

class mediamosa_node_revision {

  // ------------------------------------------------------------------ Functions (static).
  /**
   * Get the object.
   *
   * @param $nid
   * @param $vid
   */
  static public function get($nid, $vid) {
    return mediamosa_db::db_select(mediamosa_node_revision_db::TABLE_NAME, 'nr')
      ->fields('nr')
      ->condition('nid', $nid)
      ->condition('vid', $vid)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Load the nodes from the revision table.
   * @param object $nodes
   */
  static public function load(&$nodes) {
    // Gather all vids.
    $nid = $vids = array();
    foreach ($nodes as $node) {
      $vids[] = $node->vid;
      $nids[] = $node->nid;
    }

    // Gather all required vids.
    $result_vids = mediamosa_db::db_query(
      'SELECT * FROM {#table_name} WHERE #vid IN (:vids)',
      array(
        '#table_name' => mediamosa_node_revision_db::TABLE_NAME,
        '#vid' => mediamosa_node_revision_db::VID,
        ':vids' => $vids,
      )
    )->fetchAllAssoc(mediamosa_node_revision_db::VID, PDO::FETCH_ASSOC);

    // Gether latest if we miss a revision (f.e. not saved).
    $result_latest = mediamosa_db::db_query(
      'SELECT * FROM {#table_name} WHERE #nid IN (:nids) GROUP BY #nid ORDER BY #nid ASC, #vid DESC',
      array(
        '#table_name' => mediamosa_node_revision_db::TABLE_NAME,
        '#nid' => mediamosa_node_revision_db::NID,
        ':nids' => $nids,
        '#vid' => mediamosa_node_revision_db::VID,
      )
    )->fetchAllAssoc(mediamosa_node_revision_db::NID, PDO::FETCH_ASSOC);

    foreach ($nodes as $nid => $node) {

      $revision_data = array();
      if (isset($result_vids[$node->vid])) {
        $revision_data = !empty($result_vids[$node->vid][mediamosa_node_revision_db::REVISION_DATA]) ? unserialize($result_vids[$node->vid][mediamosa_node_revision_db::REVISION_DATA]) : array();
      }

      // Found it?
      if (empty($revision_data)) {
        // If we didn't find the revision, we'll take the latest version instead.
        $revision_data = !empty($result_latest[$node->nid][mediamosa_node_revision_db::REVISION_DATA]) ? unserialize($result_latest[$node->nid][mediamosa_node_revision_db::REVISION_DATA]) : array();
      }

      unset($revision_data['created']);
      unset($revision_data['changed']);

      foreach ($revision_data as $key => $value) {
        $nodes[$node->nid]->$key = $value;
      }

      // oAuth fix for earlier node rervisions.
      if ($node->type == mediamosa_node::MEDIAMOSA_NODE_TYPE_APP) {
        if (!isset($revision_data[mediamosa_app_db::OAUTH_CONSUMER_KEY]) && !isset($revision_data[mediamosa_app_db::OAUTH_CONSUMER_SECRET])) {
          $nodes[$node->nid]->{mediamosa_app_db::OAUTH_CONSUMER_KEY} = '';
          $nodes[$node->nid]->{mediamosa_app_db::OAUTH_CONSUMER_SECRET} = '';
        }
      }
    }

    return $nodes;
  }

  /**
   * Insert new revision of object.
   *
   * @param integer $nid
   * @param integer $vid
   * @param array $revision_data
   */
  static public function insert($nid, $vid, array $revision_data) {

    mediamosa_db::db_insert(mediamosa_node_revision_db::TABLE_NAME)
      ->fields(array(
        mediamosa_node_revision_db::NID => $nid,
        mediamosa_node_revision_db::VID => $vid,
        mediamosa_node_revision_db::REVISION_DATA => serialize($revision_data)
      ))
      ->execute();
  }

  /**
   * Will insert into parent table and revision table.
   *
   * @param object $node
   *  The object node to store.
   * @param string $table_name
   *  The table name where to store.
   * @param array $names
   *  The $names of the object in $node we want to store.
   */
  static public function insert_ex($node, $table_name, array $names) {
    // Collect the data we want to insert.
    $fields = array();
    foreach ($names as $name) {
      if (isset($node->{$name})) {
        $fields[$name] = $node->{$name};
      }
    }

    // Enrich.
    if (in_array(mediamosa_db::CREATED, $names) && (!isset($node->revision) || !$node->revision)) {
      $fields = mediamosa_db::db_insert_enrich($fields);
    }
    elseif (in_array(mediamosa_db::CHANGED, $names)) {
      // Update the changed time node.
      mediamosa_db::db_update('node')
        ->fields(array('changed' => REQUEST_TIME))
        ->condition('nid', $node->nid)
        ->execute();

      // Remove created from result so we dont save created.
      unset($fields[mediamosa_db::CREATED]);
      $fields = mediamosa_db::db_update_enrich($fields);
    }

    // Because revisions are stored in other table, we might have a row already.
    // Check and update if we already have one.
    if (mediamosa_db::db_select($table_name, 'tmp')->fields('tmp', array('nid'))->condition('nid', $node->nid)->execute()->fetchField()) {
      // Update.
      mediamosa_db::db_update($table_name)
        ->fields($fields)
        ->condition('nid', $node->nid)
        ->execute();
    }
    else {
      // If have a created field, then enrich with created and changed fields.
      if (in_array(mediamosa_db::CREATED, $names)) {
        $fields = mediamosa_db::db_insert_enrich($fields);
      }

      // Insert.
      mediamosa_db::db_insert($table_name)
        ->fields($fields)
        ->execute();
    }

    // Get the row so we can store the revision.
    $revision = mediamosa_db::db_select($table_name, 'tmp')
      ->fields('tmp')
      ->condition('nid', $node->nid)
      ->execute()
      ->fetchAssoc();

    // Now update with the updated fields so we change the last revision.
    foreach ($fields as $name => $value) {
      $revision[$name] = $value;
    }

    // Insert revision.
    mediamosa_node_revision::insert($node->nid, $node->vid, $revision);
  }

  /**
   * Update the revision data.
   *
   * @param $nid
   * @param $vid
   * @param $revision_data
   */
  static public function update($nid, $vid, array $revision_data) {

    mediamosa_db::db_update(mediamosa_node_revision_db::TABLE_NAME)
      ->fields(array(
        mediamosa_node_revision_db::REVISION_DATA => serialize($revision_data)
      ))
      ->condition(mediamosa_node_revision_db::NID, $nid)
      ->condition(mediamosa_node_revision_db::VID, $vid)
      ->execute();
  }

  /**
   * Will update the parent table and revision table.
   *
   * @param object $node
   *  The object node to store.
   * @param string $table_name
   *  The table name where to store.
   * @param array names
   *  The names of the fields in $node we want to store.
   */
  static public function update_ex($node, $table_name, array $names) {

    // Get the current revision (if any).
    $revision = self::get($node->nid, $node->vid);
    if (!$revision) {
      $revision = array();
    }

    // Enrich for update.
    if (in_array(mediamosa_db::CHANGED, $names)) {
      $revision = mediamosa_db::db_update_enrich($revision);
    }

    // Now create fields for update and fill revision with changes.
    $fields = array();
    foreach ($names as $name) {
      // No need to update these.
      if (in_array($name, array('nid', 'vid', 'created', 'changed'))) {
        continue;
      }

      if (isset($node->{$name}) || is_null($node->{$name})) {
        $revision[$name] = $fields[$name] = $node->{$name};
      }
    }

    // Update the changed time node.
    mediamosa_db::db_update('node')
      ->fields(array('changed' => REQUEST_TIME))
      ->condition('nid', $node->nid)
      ->execute();

    // Lets do sep.
    $revision['changed'] = $node->changed = REQUEST_TIME;

    // Update the edited revision.
    mediamosa_node_revision::update($node->nid, $node->vid, $revision);

    // Only if we find the row, then its the current version.
    mediamosa_db::db_update($table_name)
      ->fields($fields)
      ->condition('nid', $node->nid)
      ->condition('vid', $node->vid)
      ->execute();
  }

  /**
   * Delete the revisions of this node.
   * @param $nid
   */
  static public function delete($node) {
    // Remove revision.
    mediamosa_db::db_delete(mediamosa_node_revision_db::TABLE_NAME)
      ->condition(mediamosa_node_revision_db::NID, $node->nid)
      ->execute();
  }

  /**
   * Delete the revisions of this node.
   * @param $nid
   */
  static public function delete_revision($node) {

    // Remove revision.
    mediamosa_db::db_delete(mediamosa_node_revision_db::TABLE_NAME)
      ->condition(mediamosa_node_revision_db::VID, $node->vid)
      ->execute();
  }

  /**
   * Delete the revisions of this node and from parent table.
   * @param $nid
   */
  static public function delete_ex($node, $table_name = NULL) {
    // Not all types have child tables.
    if (isset($table_name)) {
      // Remove from main table.
      mediamosa_db::db_delete($table_name)
        ->condition('nid', $node->nid)
        ->execute();
    }

    // Remove revision.
    self::delete($node);
  }
}
