<?php
/**
 * @file
 * All non-hook functions are here.
 */

/**
 * Called when pressed on test button.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function _mediamosa_solr_ping_submit($form, &$form_state) {
  $mediamosa_solr_url = $form_state['values']['mediamosa_solr_url'];
  $check_connection_save = $form_state['values']['mediamosa_solr_check_connection_save'];

  // Don't want to save this in a variable.
  $_SESSION['mediamosa_solr_check_connection_save'] = $form_state['values']['mediamosa_solr_check_connection_save'];
  unset($form_state['values']['mediamosa_solr_check_connection_save']);

  if (mediamosa_solr_apache_solr_service::solrServicePing($mediamosa_solr_url)) {
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

  // Ping the solr server.
  $ping_success = mediamosa_solr_apache_solr_service::solrServicePing();

  $results = array();
  $results_config = array();
  if ($ping_success) {
    $results['solr_ping'] = array(
      'title' => t('Apache Solr servlet'),
      'value' => t('Found'),
      'severity' => _mediamosa_maintenance_status_okerror($ping_success),
    );

    // Check the schema (only 4.2. or higher).
    $schema_info = mediamosa_solr_apache_solr_service::solrServiceGetSchemaInfo();
    if ($schema_info === FALSE) {
      // Old version, can retrieve it.
      $ok = TRUE;
      $schema_info['name'] = 'No schema found, retrieving schema data is only supported by Solr 4.2 or higher.';
    }
    else {
      $ok = $schema_info['success'];
      $description = '';
      if ($ok) {
        $version = mediamosa_solr::schema_version($schema_info['name']);
        if (empty($version['build']) || $version['build'] < mediamosa_solr::MIN_BUILD) {
           $ok = FALSE;
           $description = t('Invalid Solr schema detected (build @build), please replace with the Solr files located under the MediaMosa Solr directory (schema.xml and solrconfig.xml)', array('@build' => $version['build']));
        }
      }
    }

    $results_config['solr_schema_info'] = array(
      'title' => t('Apache Solr schema'),
      'value' => $schema_info['name'],
      'severity' => _mediamosa_maintenance_status_okerror($ok),
      'description' => $description,
    );
  }
  else {
    $results['solr_ping'] = array(
      'title' => t('Apache Solr servlet'),
      'value' => t('Failed'),
      'severity' => _mediamosa_maintenance_status_okerror($ping_success),
      'description' => t('Unable to reach the Solr servlet, either !disable or check !settings.',
        array(
          '!disable' => l(t('disable the Solr module'), 'admin/modules'),
          '!settings' => l(t('the MediaMosa solr settings'), 'admin/mediamosa/config/solr')
        )
      ),
    );
  }

  return array(
    'components' => array(
      'title' => t('Components'),
      'results' => $results,
    ),
    'configuration' => array(
      'title' => t('Components'),
      'results' => $results_config,
    ),
  );
}
