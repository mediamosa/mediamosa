<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * All non-hook functions are here.
 */

/**
 * Called when pressed on test button.
 *
 * @param array $form_state
 * @param array $form
 */
function _mediamosa_solr_ping_submit($form, &$form_state) {
  $mediamosa_solr_url = $form_state['values']['mediamosa_solr_url'];
  $check_connection_save = $form_state['values']['mediamosa_solr_check_connection_save'];

  // Don't want to save this in a variable.
  $_SESSION['mediamosa_solr_check_connection_save'] = $form_state['values']['mediamosa_solr_check_connection_save'];
  unset($form_state['values']['mediamosa_solr_check_connection_save']);

  if (mediamosa_solr_apache_solr_service::mediamosaPing($mediamosa_solr_url)) {
    if ($check_connection_save) {
      drupal_set_message(t('Connection test successful, URL saved.'));

      mediamosa_solr_apache_solr_service::mediamosaSetUrl($mediamosa_solr_url);
    }
    else {
      drupal_set_message(t('Connection test successful. Warning: URL was not saved.'));
    }
  }
  else {
    drupal_set_message(t('Unable to connect to server, check URL.'), 'error');
  }
}

/**
 * Generate information for the status page.
 */
function _mediamosa_solr_status_generate() {
  // Make sure this module is enabled.
  if (!module_exists('mediamosa_maintenance_status')) {
    return array();
  }

  $ping_success = mediamosa_solr_apache_solr_service::mediamosaPing();

  if ($ping_success) {
    $results['solr_ping'] = array(
      'title' => t('Lucene Solr servlet'),
      'value' => t('Found'),
      'severity' => _mediamosa_maintenance_status_okerror($ping_success),
    );
  }
  else {
    $results['solr_ping'] = array(
      'title' => t('Lucene Solr servlet'),
      'value' => t('Failed'),
      'severity' => _mediamosa_maintenance_status_okerror($ping_success),
      'description' => t('Unable to reach the Solr servlet, either !disable or check !settings.',
        array(
          '!disable' => l('disable the Solr module', 'admin/modules'),
          '!settings' => l('the MediaMosa solr settings', 'admin/mediamosa/config/solr')
        )
      ),
    );
  }

  return array(
    'components' => array(
      'title' => t('Components'),
      'results' => $results,
    ),
  );
}
