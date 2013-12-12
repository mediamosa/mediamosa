<?php
/**
 * @file
 * hook_load / -update / -delete / -insert hooks for node type mediamosa_viewer.
 *
 * There is no need to revision the tools and containers for transcode or
 * streaming server. These values are taken from one field in the main object.
 */

/**
 * Implements hook_load().
 */
function mediamosa_viewer_load(&$nodes) {
  mediamosa_node_revision::load($nodes);
  return $nodes;
}

/**
 * Implements hook_update().
 */
function mediamosa_viewer_update($node) {
  if ($node->is_new || !empty($node->revision)) {
    mediamosa_viewer_insert($node);
  }
  else {
    // Get the field names of the table.
    $names = mediamosa_viewer_db::get_fields();

    // Copy title.
    $node->{mediamosa_viewer_db::NAME} = $node->title;

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_viewer_db::TABLE_NAME, $names);
  }
}

/**
 * Implements hook_insert().
 */
function mediamosa_viewer_insert(&$node) {
  // Get the field names of the table.
  $names = mediamosa_viewer_db::get_fields();

  // Copy title.
  $node->{mediamosa_viewer_db::NAME} = $node->title;

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_viewer_db::TABLE_NAME, $names);
}

/**
 * Implements hook_delete().
 */
function mediamosa_viewer_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete_ex($node, mediamosa_viewer_db::TABLE_NAME);
}

/**
 * Implements hook_node_revision_delete().
 */
function mediamosa_viewer_node_revision_delete($node) {
  mediamosa_node_revision::delete_revision($node);
}

/**
 * Implements hook_view().
 */
function mediamosa_viewer_view($node, $view_mode = 'full') {
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Viewer listing'), 'admin/mediamosa/config/viewer');
  drupal_set_breadcrumb($breadcrumb);

  // Default parameters.
  $rows = array(
    array(t('Name'), check_plain($node->title)),
    array(t('Description'), $node->description),
    array(t('Object Code'), '<pre>' . check_plain($node->object_code) . '</pre>'),
    array(t('Created'), format_date($node->created, 'short')),
    array(t('Changed'), format_date($node->changed, 'short'))
  );

  $node->content['app'] = array(
    '#markup' => theme('table', array('header' => array(t('Parameter'), t('Value')), 'rows' => $rows)),
  );

  return $node;
}

/**
 * Implements hook_form().
 */
function mediamosa_viewer_form($node, &$form_state) {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Viewer listing'), 'admin/mediamosa/config/viewer');
  drupal_set_breadcrumb($breadcrumb);

  $is_insert = empty($node->nid);

  $title = $is_insert ? t('New viewer') : t("Editing viewer server '@viewer_name'.", array('@viewer_name' => $node->title));

  $form = array();

  $form['mediamosa_viewer'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => $title,
    '#description' => '',
  );

  $form['mediamosa_viewer']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Viewer name'),
    '#description' => t('The unique name for the viewer.'),
    '#required' => TRUE,
    '#default_value' => mediamosa_node::default_value('title', $node),
    '#maxlength' => mediamosa_viewer_db::NAME_LENGTH,
  );

  $form['mediamosa_viewer'][mediamosa_viewer_db::DESCRIPTION] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Description of the server.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_viewer_db::DESCRIPTION, $node),
    '#maxlength' => mediamosa_viewer_db::DESCRIPTION_LENGTH,
  );

  $form['mediamosa_viewer'][mediamosa_viewer_db::OBJECT_CODE] = array(
    '#type' => 'textarea',
    '#title' => t('Object code'),
    '#required' => TRUE,
    '#description' => t('{WIDTH}, {HEIGHT}, {HEIGHT_PLUS_number}, {MEDIAFILE_ID}, {TICKET_URI}, {STILL_URI}, {AUTOPLAY} (true or false), {AUTOPLAY_NUM} (1 or 0), {AUTOPLAY_TEXT} (autostart), {IF_START}, {/IF_START}, {IF_EXTERNAL}, {/IF_EXTERNAL}, {IF_DURATION}, {/IF_DURATION} are available for use in this field.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_viewer_db::OBJECT_CODE, $node),
  );

  $form['#redirect'] = 'admin/mediamosa/config/viewer';

  return $form;
}

/**
 * Implements hook_validate().
 */
function mediamosa_viewer_validate($node) {
}
