<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * The settings are stored inside a node for its revisions.
 */

class mediamosa_node_mediamosa_settings {

  /**
   * Helper tool to collect the variable names in a form.
   *
   * @param array $form
   * @param array $names
   */
  static public function collect_variable_names_from_form(array $form, array &$names) {

    if (!is_array($form)) {
      return;
    }

    foreach ($form as $key => $child_form) {
      if ($key[0] == '#') {
        continue;
      }

      // If where we point is array and a form (#type must be here), then add
      // as variable name.
      if (is_array($child_form) && isset($child_form['#type'])) {
        switch ($child_form['#type']) {
          case 'checkbox':
          case 'checkboxes':
          case 'date':
          case 'file':
          case 'password':
          case 'radio':
          case 'radios':
          case 'select':
          case 'textarea':
          case 'textfield':
            $names[] = $key;
            break;

          default:
            self::collect_variable_names_from_form($child_form, $names);
        }
      }
      else {
        self::collect_variable_names_from_form($child_form, $names);
      }
    }
  }

  /**
   * Return the field names use in the settings variables.
   */
  static public function get_fields() {
    // Get the settings form.
    $form = module_invoke_all('mediamosa_configuration_collect');

    $names = array();

    // Collect the variable names.
    self::collect_variable_names_from_form($form, $names);

    // Now return the names as fields.
    return $names;
  }

  /**
   * Return the node with settings. If non is found, it will create a dummy.
   */
  static public function get_node_settings() {

    // I just need the nid, so i can load the node.
    $nid = mediamosa_db::db_select('node', 'n')
      ->fields('n', array('nid'))
      ->condition('type', mediamosa_node::MEDIAMOSA_NODE_TYPE_SETTINGS)
      ->range(0, 1)
      ->orderBy('nid', 'ASC')
      ->execute()->fetchField();

    // Found?
    if (!$nid) {
      $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_SETTINGS, 'MediaMosa Configuration');
      $node->new = 1;
      return $node;
    }

    // Return the node.
    return node_load($nid);
  }
}

/**
 * Implements hook_load().
 */
function mediamosa_settings_load(&$nodes) {
  return mediamosa_node_revision::load($nodes);
}

/**
 * Implements hook_update().
 */
function mediamosa_settings_update($node) {

  if ($node->revision) {
    mediamosa_settings_insert($node);
  }
  else {
    // Get the field names of the table.
    $names = mediamosa_node_mediamosa_settings::get_fields();

    $revision = array();
    foreach ($names as $name) {
      $revision[$name] = isset($node->{$name}) ? $node->{$name} : variable_get($name);
    }

    // Update the edited revision.
    mediamosa_node_revision::update($node->nid, $node->vid, $revision);
  }
}

/**
 * Implements hook_insert().
 */
function mediamosa_settings_insert($node) {
  // Get the field names of the table.
  $names = mediamosa_node_mediamosa_settings::get_fields();

  $revision = array();
  foreach ($names as $name) {
    // Suppling revision?
    if ($node->revision) {
      $revision[$name] = $node->{$name};
      variable_set($name, $node->{$name});
    }
    else {
      $revision[$name] = variable_get($name);
    }
  }

  // Insert revision.
  mediamosa_node_revision::insert($node->nid, $node->vid, $revision);
}

/**
 * Implements hook_node_revision_delete().
 */
function mediamosa_settings_node_revision_delete($node) {
  mediamosa_node_revision::delete_revision($node);
}

/**
 * Implements hook_delete().
 */
function mediamosa_settings_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete_ex($node);
}

/**
 * Implements hook_view().
 */
function mediamosa_settings_view($node, $view_mode = 'full') {
  drupal_goto('admin/mediamosa/config/global');
}

/**
 * Implements hook_view().
 */
function mediamosa_settings_form($node, $view_mode = 'full') {
  drupal_goto('admin/mediamosa/config/global');
}
