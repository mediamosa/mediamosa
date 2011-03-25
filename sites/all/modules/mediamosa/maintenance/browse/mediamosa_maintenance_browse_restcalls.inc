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
  * Browser of REST calls.
  */

/**
 * The main browse form.
 */
function mediamosa_maintenance_browse_restcall_form() {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

  // Call the REST call hook to get the registered REST calls.
  $rest_calls = module_invoke_all('mediamosa_register_rest_call');
  $rest_calls_doc = module_invoke_all('mediamosa_register_rest_call_doc');
  $rest_calls = array_merge_recursive($rest_calls, $rest_calls_doc);

  $item_count_total = 0;

  // Skip internal calls, but count the rest.
  foreach ($rest_calls as $key => $rest_call) {
    foreach ($rest_call as $method => $value) {

      if (isset($value['access']) && ($value['access'] == mediamosa_rest_call::ACCESS_INTERNAL_ONLY)) {
        unset($rest_calls[$key][$method]);
      }
      elseif (isset($value['status']) && ($value['status'] == 'deprecated')) { // @todo make switch for admin to select to show them.
        unset($rest_calls[$key][$method]);
      }
      else {
        $item_count_total++;
      }
    }
  }

  // Sort the rest calls on URI.
  ksort($rest_calls);

  $item_limit = mediamosa_settings::maintenance_items_per_page();
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $offset = $page * $item_limit;

  $header = array(
    array('data' => t('URI')),
    array('data' => t('Method')),
    array('data' => t('Status')),
    array('data' => t('Version')),
  );

  $form = $rows = array();

  $pos = 0;
  foreach ($rest_calls as $uri => $rest_calls_2) {

    if ($pos >= ($offset + $item_limit)) {
      break;
    }

    ksort($rest_calls_2);
    $has_warnings = FALSE;
    foreach ($rest_calls_2 as $method => $rest_call) {

      // Because we get all the REST calls, we need to skip some before we start.
      if ($pos < $offset) {
        $pos++;
        continue;
      }
      elseif ($pos >= ($offset + $item_limit)) {
        break;
      }

      $pos++;

      $has_example_request = !empty($rest_call[mediamosa_rest_call::EXAMPLE_REQUEST]);
      $has_example_response = !empty($rest_call[mediamosa_rest_call::EXAMPLE_RESPONSE]) || (isset($rest_call[mediamosa_rest_call::EXAMPLE_RESPONSE]) && $rest_call[mediamosa_rest_call::EXAMPLE_RESPONSE] === FALSE);

      $warnings = array();
      if (!$has_example_request) {
        $has_warnings = TRUE;
        $warnings[] = '<sup>1</sup>';
      }
      if (!$has_example_response) {
        $has_warnings = TRUE;
        $warnings[] = '<sup>2</sup>';
      }

      $rows[] = array(
        l(t('/' . $uri), 'admin/mediamosa/browse/restcall/' . str_replace('/', '-', $uri) . '/' . urlencode($method)) . ' ' . implode(' ', $warnings),
        $method,
        $rest_call[mediamosa_rest_call::STATUS],
        $rest_call[mediamosa_rest_call::VERSION],
      );
    }
  }

  // Our listing.
  $form['list'] = array(
    '#theme' => 'mediamosa_maintenance_table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No REST calls found.'),
    // Pager.
    '#page' => $page,
    '#item_count_total' => $item_count_total,
    '#item_limit' => $item_limit,
    // Note
    '#note' => $has_warnings ? implode(', ', array(t('1) missing example request'), t('2) missing example response'))) : '',
    // Total note.
    '#caption_bottom' => t('Total found: @total', array('@total' => $item_count_total))
  );

  return $form;
}

/**
 * View specific details of a REST call.
 */
function mediamosa_maintenance_browse_restcall_details($uri, $method) {
  // If we are set as Home page, the breadcrumb isnt set, lets do it here.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  $breadcrumb[] = l(t('Rest calls browser'), 'admin/mediamosa/browse/restcall');
  drupal_set_breadcrumb($breadcrumb);

  $uri = str_replace('-', '/', $uri);
  $rest_calls = module_invoke_all('mediamosa_register_rest_call');
  $rest_calls_doc = module_invoke_all('mediamosa_register_rest_call_doc');
  $rest_calls = array_merge_recursive($rest_calls, $rest_calls_doc);

  if (!isset($rest_calls[$uri][$method])) {
    return t('Rest call not found.');
  }

  // Add URI and Method to the array.
  $rest_calls[$uri][$method][mediamosa_rest_call::URI] = $uri;
  $rest_calls[$uri][$method][mediamosa_rest_call::METHOD] = $method;

  // Enrich the rest call with missing defaults.
  $a_rest_call = mediamosa_rest::set_default_rest_call($rest_calls[$uri][$method]);

  // Create class from it.
  $rest_call = new $rest_calls[$uri][$method][mediamosa_rest_call::CLASS_NAME]($a_rest_call);
  $rest_call->{mediamosa_rest_call::CLASS_NAME} = $rest_calls[$uri][$method][mediamosa_rest_call::CLASS_NAME];

  // Check if override for class selection is here.
  if (method_exists($rest_call, 'get_object_rest_call')) {
    $rest_call = $rest_call->get_object_rest_call($a_rest_call, isset($a_rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES]) ? $a_rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES] : array());
  }

  // Set as uri values, so we dont fail checks.
  if (isset($rest_calls[$uri][$method][mediamosa_rest_call::DEFAULT_PARAMS_VALUES])) {
    foreach ($rest_calls[$uri][$method][mediamosa_rest_call::DEFAULT_PARAMS_VALUES] as $param => $value) {
      $rest_call->{mediamosa_rest_call::URI_PARAMS}[$param] = $value;
    }
  }

  // Turn off so we can call the get_var_setup without problems.
  // Although get_var_setup should not test variables.
  $rest_call->set_check_for_unspecified(FALSE);

  // Get the var setup for this call.
  $var_setup = $rest_call->get_var_setup();

  // Setup the title with the REST call URI.
  drupal_set_title(
    t('Viewing details rest call URI @uri, method [@method]',
      array(
      '@uri' => '/' . $uri,
      '@method' => $method
      )
    )
  );

  $warnings = array();

  if ($rest_call->access >= mediamosa_rest_call::ACCESS_AUTHENTICATED && ($rest_call->access & mediamosa_rest_call::ACCESS_EXTERNAL) != mediamosa_rest_call::ACCESS_EXTERNAL) {
    if (!isset($var_setup['vars']['app_id'])) {
      $warnings[] = 'Check vars, missing app_id.';
    }
  }

  // We dont want to show the app_id to the outside.
  unset($var_setup['vars']['app_id']);

  if ((isset($var_setup['vars']['is_app_admin'])) && ($var_setup['vars']['is_app_admin']['description'] == 'NOT USED.')) {
    unset($var_setup['vars']['is_app_admin']);
  }

  $form = array();

  $form['restcall_doc_template'] = array(
    '#theme' => 'restcall_doc',
    '#rest_call' => $rest_call,
    '#var_setup' => $var_setup,
    '#warnings' => $warnings,
  );

  return $form;
}

/**
 * Process variables for restcall-doc.tpl.php.
 *
 * @see restcall-doc.tpl.php
 */
function template_preprocess_restcall_doc(&$variables) {
  $rest_call = $variables['rest_call'];
  $var_setup = $variables['var_setup'];

  $variables['rest_call']->request_authorization = t('This call can be called without any authentication.');

  if ($rest_call->access >= mediamosa_rest_call::ACCESS_AUTHENTICATED && empty($rest_call->{mediamosa_rest_call::BEHAVE_AS_EXTERNAL})) {
    $variables['rest_call']->request_authorization = t('This call needs EGA authentication.');
  }

  $header = array(
    array('data' => t('Parameter')),
    array('data' => t('Type')),
    array('data' => t('Description')),
    array('data' => t('Required')),
    array('data' => t('Default value')),
    array('data' => t('Max. length')),
  );

  // Sort on param.
  ksort($var_setup[mediamosa_rest_call::VARS]);
  $form = $rows = array();

  if (!isset($var_setup[mediamosa_rest_call::VARS])) {
    $var_setup[mediamosa_rest_call::VARS] = array();
  }
  // First required.
  foreach ($var_setup[mediamosa_rest_call::VARS] as $param => $item) {
    if (isset($item[mediamosa_rest_call::VAR_IS_REQUIRED]) && $item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_YES) {
      $is_array = isset($item[mediamosa_rest_call::VAR_IS_ARRAY]) && $item[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES ? '[]' : '';

      // Get allowed values.
      $allowed_values = !empty($item[mediamosa_rest_call::VAR_ALLOWED_VALUES]) ? $item[mediamosa_rest_call::VAR_ALLOWED_VALUES] : array();

      $rows[] = array(
        $param . $is_array,
        mediamosa_sdk::typeAlias2Type($item[mediamosa_rest_call::VAR_TYPE]),
        (isset($item[mediamosa_rest_call::VAR_DESCRIPTION]) ? $item[mediamosa_rest_call::VAR_DESCRIPTION] : '') . (!empty($allowed_values) ? '<br /><small>Value must be either "' . implode('" or "', $allowed_values) . '".' : ''),
        t('Required'),
        isset($item[mediamosa_rest_call::VAR_DEFAULT_VALUE]) ? $item[mediamosa_rest_call::VAR_DEFAULT_VALUE] : '',
        isset($item[mediamosa_rest_call::VAR_RANGE_END]) ? $item[mediamosa_rest_call::VAR_RANGE_END] : '-',
      );
    }
  }

  // Not required.
  foreach ($var_setup[mediamosa_rest_call::VARS] as $param => $item) {
    if (!isset($item[mediamosa_rest_call::VAR_IS_REQUIRED]) || $item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_NO) {
      $is_array = isset($item[mediamosa_rest_call::VAR_IS_ARRAY]) && $item[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES ? '[]' : '';

      // Get allowed values.
      $allowed_values = !empty($item[mediamosa_rest_call::VAR_ALLOWED_VALUES]) ? $item[mediamosa_rest_call::VAR_ALLOWED_VALUES] : array();

      $rows[] = array(
        $param . $is_array,
        mediamosa_sdk::typeAlias2Type($item[mediamosa_rest_call::VAR_TYPE]),
        (isset($item[mediamosa_rest_call::VAR_DESCRIPTION]) ? $item[mediamosa_rest_call::VAR_DESCRIPTION] : '') . (!empty($allowed_values) ? '<br /><small>Value (when supplied) must be either "' . implode('" or "', $allowed_values) . '".' : ''),
        t('Optional'),
        isset($item[mediamosa_rest_call::VAR_DEFAULT_VALUE]) ? $item[mediamosa_rest_call::VAR_DEFAULT_VALUE] : '',
        isset($item[mediamosa_rest_call::VAR_RANGE_END]) ? $item[mediamosa_rest_call::VAR_RANGE_END] : '-',
      );
    }
  }

  // Parameter list.
  $variables['parameters'] = theme(
    'table',
    array(
      'header' => $header,
      'rows' => $rows,
      'empty' => t('This REST call has no input parameters.'),
    )
  );

  // Response fields.
  $header = array(
    array('data' => t('Field')),
    array('data' => t('Description')),
  );

  if (!empty($rest_call->response_fields)) {
    $rows = array();
    foreach ($rest_call->response_fields as $key => $value) {
      $rows[] = array($key, $value);
    }

    // Hide table when response fields is empty.
    if (!empty($rows)) {
      $variables['response_fields'] = theme(
        'table',
        array(
          'header' => $header,
          'rows' => $rows,
          'empty' => t('No response fields found.')
        )
      );
    }
  }
  elseif ($rest_call->response_fields === FALSE) {
    $variables['response_fields'] = t('This REST call does not have any response fields and will always return empty items array');
  }

  if (empty($variables['rest_call']->example_request)) {
    $variables['warnings'][] = t('No example request specified.');
  }
  if ($variables['rest_call']->example_response === FALSE) {
    if ($rest_call->{mediamosa_rest_call::METHOD} == mediamosa_rest_call::METHOD_GET) {
      $variables['rest_call']->example_response = t('This REST call does not generate a items response, check the result ID instead. Will return 601 (OK) or 705 (empty result) as success.');
    }
    else {
      $variables['rest_call']->example_response = t('This REST call does not generate a items response, check the result ID instead. Will return 601 (OK) as success.');
    }
  }

  if (empty($variables['rest_call']->example_response)) {
    $variables['warnings'][] = t('No example response specified.');
  }
}
