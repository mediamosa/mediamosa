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
 * Browse the error codes.
 */

/**
 * The main browse form.
 */
function mediamosa_maintenance_browse_errorcode_form() {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

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
      $actions[] = l(t('Edit'), 'admin/mediamosa/browse/errorcode/' . $errorcode[mediamosa_error_db::ID] . '/edit', $l_options);
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
  // If we are set as Home page, the breadcrumb isnt set, lets do it here.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  $breadcrumb[] = l(t('Errorcode management'), 'admin/mediamosa/browse/errorcode');
  drupal_set_breadcrumb($breadcrumb);

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
