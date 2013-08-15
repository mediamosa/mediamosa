<?php

/**
 * @file
 * Browse the error codes.
 */

/**
 * The main browse form.
 */
function mediamosa_maintenance_browse_errorcode_form() {

  $header = array(
    array('data' => t('ID'), 'field' => mediamosa_error_db::ID),
    array('data' => t('Name'), 'field' => mediamosa_error_db::NAME),
    array('data' => t('Code'), 'field' => mediamosa_error_db::CODE),
    array('data' => t('Operations')),
  );

  $query = mediamosa_db::db_select(mediamosa_error_db::TABLE_NAME, 'e')->extend('PagerDefault')->extend('TableSort');
  $query->fields('e');
  $query->orderByHeader($header);
  $query->limit(mediamosa_settings::maintenance_items_per_page());
  $result = $query->execute()->fetchAll();

  $form = $rows = array();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  // Build the rows.
  foreach ($result as $errorcode) {
    $actions = array();

    if (_mediamosa_user_access(mediamosa_permission::ACCESS_EDIT_ERRORCODES)) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/content/errorcode/' . $errorcode[mediamosa_error_db::ID] . '/edit', $l_options);
    }

    $rows[] = array('data' =>
      array(
        // Cells
        $errorcode[mediamosa_error_db::ID],
        check_plain($errorcode[mediamosa_error_db::NAME]) . '<br /><small>' . check_plain($errorcode[mediamosa_error_db::MESSAGE]) . '</small>',
        check_plain($errorcode[mediamosa_error_db::CODE]),
        empty($actions) ? '-' :implode(' | ', $actions)
      ),
    );
  }

  $form['log_pager_top'] = array('#theme' => 'pager');
  $form['log_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );
  $form['log_pager_bottom'] = array('#theme' => 'pager');

  return $form;
}

/**
 * The edit form.
 */
function mediamosa_maintenance_browse_errorcode_edit_form($form, &$form_state, $id) {
  $form = array();

  $errorcode = mediamosa_error::get($id);

  // Settings for anonymous users.
  $form['errorcode'] = array(
    '#type' => 'fieldset',
    '#title' => t('MediaMosa Errorcode'),
  );
  $form[mediamosa_error_db::ID] = array(
    '#type' => 'hidden',
    '#default_value' => $errorcode[mediamosa_error_db::ID],
  );
  $form['errorcode'][mediamosa_error_db::NAME] = array(
    '#type' => 'item',
    '#title' => t('Name'),
    '#description' => $errorcode[mediamosa_error_db::NAME],
  );
  $form['errorcode'][mediamosa_error_db::CODE] = array(
    '#type' => 'item',
    '#title' => t('Code'),
    '#description' => $errorcode[mediamosa_error_db::CODE]
  );
  $form['errorcode'][mediamosa_error_db::MESSAGE] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#default_value' => $errorcode[mediamosa_error_db::MESSAGE],
    '#description' => t('Message to be displayed in case this error occurs.'),
  );

  $form['errorcode']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );


  return $form;
}

/**
 * Submit the errorcode update form.
 */
function mediamosa_maintenance_browse_errorcode_edit_form_submit($form, &$form_state) {

  $id = $form_state['values'][mediamosa_error_db::ID];
  $message = $form_state['values'][mediamosa_error_db::MESSAGE];

  // Save.
  mediamosa_db::db_update(mediamosa_error_db::TABLE_NAME)
    ->fields(array(
      mediamosa_error_db::MESSAGE => $message
    ))
    ->condition(mediamosa_error_db::ID, $id)
    ->execute();

  drupal_set_message(t('The errorcode has been saved.'));
}
