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
  * hook_load / -update / -delete / -insert hooks for node type mediamosa_tool_mapping.
  *
  *
  */

/**
 * Implements hook_load().
 */
function mediamosa_tool_mapping_load(&$nodes) {
  $nodes = mediamosa_node_revision::load($nodes);

  return $nodes;
}

/**
 * Implements hook_update().
 */
function mediamosa_tool_mapping_update($node) {

  if ($node->revision) {
    mediamosa_tool_mapping_insert($node);
  }
  else {
    // Get the field names of the table.
    $names = mediamosa_tool_mapping_db::get_fields();

    // Exception is title.
    $node->{mediamosa_tool_mapping_db::PARAMETER} = $node->title;

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_tool_mapping_db::TABLE_NAME, $names);
  }
}

/**
 * Implements hook_insert().
 */
function mediamosa_tool_mapping_insert($node) {
  // Get the field names of the table.
  $names = mediamosa_tool_mapping_db::get_fields();

  // Exception is title.
  $node->{mediamosa_tool_mapping_db::PARAMETER} = $node->title;

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_tool_mapping_db::TABLE_NAME, $names);
}

/**
 * Implements hook_delete().
 */
function mediamosa_tool_mapping_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete_ex($node, mediamosa_tool_mapping_db::TABLE_NAME);
}

/**
 * Implements hook_node_revision_delete().
 */
function mediamosa_tool_mapping_node_revision_delete($node) {
  mediamosa_node_revision::delete_revision($node);
}

/**
 * Implements hook_view().
 */
function mediamosa_tool_mapping_view($node, $view_mode = 'full') {
  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Tool metadata mapping'), 'admin/mediamosa/config/tool_mapping');
  drupal_set_breadcrumb($breadcrumb);

  // Default parameters.
  $rows = array(
    array(t('Name'), check_plain($node->parameter)),
    array(t('Tool'), check_plain($node->tool)),
    array(t('Asset property'), check_plain(mediamosa_tool_mapping::get_asset_metadata_display_name($node->prop_id))),
  );

  // Add timestamps.
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['tool_mapping'] = array(
    '#markup' => theme('table', array('header' => array(t('Parameter'), t('Value')), 'rows' => $rows)),
  );

  return $node;
}

/**
 * Implements hook_form().
 */
function mediamosa_tool_mapping_form($node, $form_state) {
  // If we're inserting a new node, set some defaults:
  if (!isset($node->nid)) {
    $node->tool = 'pdf';
    $node->required = 'FALSE';
  }

  $form = array();

  if (isset($node->tool_mapping_id)) {
    $form['tool_mapping_id'] = array(
      '#type' => 'hidden',
      '#value' => $node->tool_mapping_id,
    );
  }

  $form['title'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#description' => t('The name of this parameter.'),
    '#default_value' => mediamosa_node::default_value('title', $node),
    '#maxlength' => mediamosa_tool_mapping_db::PARAMETER_LENGTH,
  );

  $tools = mediamosa_server_tool::get_server_tools();
  $options = array();
  foreach ($tools as $tool_id => $tool) {
    $options[$tool_id] = $tool['name'];
  }

  $form[mediamosa_tool_mapping_db::TOOL] = array(
    '#title' => t('Tool'),
    '#type' => 'select',
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_params_db::TOOL, $node),
    '#options' => $options,
    '#required' => TRUE,
    '#description' => t('Select one of the available tools.'),
  );

  $options = array();
  foreach (mediamosa_asset_metadata_property::get_all_metadata() as $metadata) {
    $options[$metadata['propdef_id']] = mediamosa_tool_mapping::get_asset_metadata_display_name($metadata['propdef_id']);
  }
  $form[mediamosa_tool_mapping_db::PROP_ID] = array(
    '#title' => t('Asset property'),
    '#type' => 'select',
    '#required' => TRUE,
    '#description' => t('The parameter mapped to this asset property in case of mediafile analyse.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_mapping_db::PROP_ID, $node),
    '#options' => $options,
  );

  return $form;
}

/**
 * Implements hook_validate().
 */
function mediamosa_tool_mapping_validate($node) {
  // Tool.
  $tools = mediamosa_server_tool::get_server_tools();
  if (empty($tools[$node->{mediamosa_tool_mapping_db::TOOL}])) {
    form_set_error(mediamosa_tool_mapping_db::TOOL, t('Not existing or enabled tool.'));
  }

  if (!$node->title) {
    form_set_error('title', t('Title can not be empty.'));
  }

  // Prop_id value.
  if ($node->{mediamosa_tool_mapping_db::PROP_ID} && !mediamosa_asset_metadata_property::property_get_row($node->{mediamosa_tool_mapping_db::PROP_ID})) {
    form_set_error(mediamosa_tool_mapping_db::PROP_ID, t('Not existing property.'));
  }

  $result = mediamosa_tool_mapping::get_all();
  foreach ($result as $record) {
    if (
      $node->{mediamosa_tool_mapping_db::TOOL} == $record[mediamosa_tool_mapping_db::TOOL] &&
      $node->{mediamosa_tool_mapping_db::PROP_ID} == $record[mediamosa_tool_mapping_db::PROP_ID] &&
      $node->title != $record[mediamosa_tool_mapping_db::PARAMETER]
    ) {
      form_set_error(mediamosa_tool_mapping_db::PROP_ID, t('This property and tool already exists: @name.', array('@name' => $record[mediamosa_tool_mapping_db::PARAMETER],)));
    }
  }
}
