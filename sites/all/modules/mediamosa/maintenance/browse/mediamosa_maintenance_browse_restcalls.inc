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
 * Browser of REST calls.
 */

/**
 * The main REST call browse page.
 */
function _mediamosa_maintenance_browse_restcall_page() {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

  // Add our CSS.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance_browse') . '/mediamosa_maintenance_browse_restcalls.css');

  $build = array();

  $build['restcalls_filter'] = drupal_get_form('mediamosa_maintenance_browse_restcall_filter_form');
  $build['restcalls_list'] = drupal_get_form('mediamosa_maintenance_browse_restcall_list_form');

  return $build;
}

function _mediamosa_maintenance_browse_restcalls_filters() {

  // Collect the versions.
  $versions = array();

  // Need restcalls for versions.
  $rest_calls = _mediamosa_maintenance_browse_restcalls_get();

  // Collect versions.
  foreach ($rest_calls as $uri => $method_rest_call) {
    foreach ($method_rest_call as $method => $rest_call) {
      $versions[$rest_call[mediamosa_rest_call::VERSION]] = $rest_call[mediamosa_rest_call::VERSION];
    }
  }

  ksort($versions);

  return array(
    mediamosa_rest_call::STATUS => array(
      'title' => t('Status'),
      'options' => array(mediamosa_rest_call::STATUS_ACTIVE => t('active'), mediamosa_rest_call::STATUS_DEPRECATED => t('deprecated'), mediamosa_rest_call::STATUS_DISABLED => t('disabled')),
      'default' => array(mediamosa_rest_call::STATUS_ACTIVE => mediamosa_rest_call::STATUS_ACTIVE),
    ),
    mediamosa_rest_call::METHOD => array(
      'title' => t('Method'),
      'options' => array(
        mediamosa_rest_call::METHOD_DELETE => t('DELETE'),
        mediamosa_rest_call::METHOD_GET => t('GET'),
        mediamosa_rest_call::METHOD_POST => t('POST'),
        mediamosa_rest_call::METHOD_PUT => t('PUT'),
      ),
      'default' => array(
        mediamosa_rest_call::METHOD_DELETE => mediamosa_rest_call::METHOD_DELETE,
        mediamosa_rest_call::METHOD_GET => mediamosa_rest_call::METHOD_GET,
        mediamosa_rest_call::METHOD_POST => mediamosa_rest_call::METHOD_POST,
        mediamosa_rest_call::METHOD_PUT => mediamosa_rest_call::METHOD_PUT,
      ),
    ),
    mediamosa_rest_call::VERSION => array(
      'title' => t('Version'),
      'options' => $versions,
      'default' => $versions,
    ),
    mediamosa_rest_call::FOR_INTERFACE => array(
      'title' => t('Interface'),
      'options' => array(
        mediamosa_rest_call::FOR_INTERFACE_APP => t('App'),
        mediamosa_rest_call::FOR_INTERFACE_DOWNLOAD => t('Download'),
        mediamosa_rest_call::FOR_INTERFACE_OPENAPI => t('OpenAPI'),
        mediamosa_rest_call::FOR_INTERFACE_UPLOAD => t('Upload'),
      ),
      'default' => array(
        mediamosa_rest_call::FOR_INTERFACE_APP => mediamosa_rest_call::FOR_INTERFACE_APP,
        mediamosa_rest_call::FOR_INTERFACE_DOWNLOAD => mediamosa_rest_call::FOR_INTERFACE_DOWNLOAD,
        mediamosa_rest_call::FOR_INTERFACE_OPENAPI => mediamosa_rest_call::FOR_INTERFACE_OPENAPI,
        mediamosa_rest_call::FOR_INTERFACE_UPLOAD => mediamosa_rest_call::FOR_INTERFACE_UPLOAD,
      ),
    ),
  );
}

/**
 * Return the default setup of the filters.
 */
function _mediamosa_maintenance_browse_restcalls_filter_default() {
  $mediamosa_restcalls_filter = array();
  foreach (_mediamosa_maintenance_browse_restcalls_filters() as $name => $filter) {
    $mediamosa_restcalls_filter[$name] = $filter['default'];
  }

  // Others.
  $mediamosa_restcalls_filter['uri'] = '';

  return $mediamosa_restcalls_filter;
}

/**
 * The filter REST call browse form.
 *
 * @ingroup forms
 * @see mediamosa_maintenance_browse_restcall_filter_form_submit()
 * @see mediamosa_maintenance_browse_restcall_filter_form_validate()
 */
function mediamosa_maintenance_browse_restcall_filter_form($form) {

  // Get the filters.
  $filters = _mediamosa_maintenance_browse_restcalls_filters();

  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter REST calls'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($_SESSION['mediamosa_restcalls_filter']) || $_SESSION['mediamosa_restcalls_filter'] == _mediamosa_maintenance_browse_restcalls_filter_default(),
  );

  foreach ($filters as $key => $filter) {
    $form['filters']['restcall']['filters'][$key] = array(
      '#title' => $filter['title'],
      '#type' => 'select',
      '#multiple' => TRUE,
      '#size' => 6,
      '#options' => $filter['options'],
      '#default_value' => empty($_SESSION['mediamosa_restcalls_filter'][$key]) ? $filter['default'] : $_SESSION['mediamosa_restcalls_filter'][$key],
    );
  }

  $form['filters']['restcall']['uri'] = array(
    '#type' => 'textfield',
    '#title' => t('URI'),
    '#description' => t('Enter part of uri to filter.'),
    '#size' => 40,
    '#maxlength' => 100,
    '#required' => FALSE,
    '#default_value' => empty($_SESSION['mediamosa_restcalls_filter']['uri']) ? '' : $_SESSION['mediamosa_restcalls_filter']['uri'],
  );

  $form['filters']['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('container-inline')),
  );
  $form['filters']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
  );

  if (!empty($_SESSION['mediamosa_restcalls_filter']) && $_SESSION['mediamosa_restcalls_filter'] != _mediamosa_maintenance_browse_restcalls_filter_default()) {
    $form['filters']['actions']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset')
    );
  }
  return $form;
}

/**
 * Validate result from the filter form.
 */
function mediamosa_maintenance_browse_restcall_filter_form_validate($form, &$form_state) {
  if ($form_state['values']['op'] == t('Filter') && empty($form_state['values']['status']) && empty($form_state['values']['uri'])) {
    form_set_error('type', t('You must select something to filter by.'));
  }
}

/**
 * Process result from the filter form.
 */
function mediamosa_maintenance_browse_restcall_filter_form_submit($form, &$form_state) {

  // Get the filters.
  $filters = _mediamosa_maintenance_browse_restcalls_filters();

  $op = $form_state['values']['op'];
  switch ($op) {
    case t('Filter'):
      $_SESSION['mediamosa_restcalls_filter']['uri'] = trim($form_state['values']['uri']);
      // Set default or value.
      foreach ($filters as $name => $filter) {
        $_SESSION['mediamosa_restcalls_filter'][$name] = empty($form_state['values'][$name]) ? $filter['default'] : $form_state['values'][$name];
      }
      break;

    case t('Reset'):
      $_SESSION['mediamosa_restcalls_filter'] = _mediamosa_maintenance_browse_restcalls_filter_default();
      break;
  }
  return 'admin/reports/dblog';
}

/**
 * Get the REST calls.
 */
function _mediamosa_maintenance_browse_restcalls_get() {

  $rest_calls = &drupal_static(__FUNCTION__, array());
  if (!empty($rest_calls)) {
    return $rest_calls;
  }

  // Call the REST call hook to get the registered REST calls.
  $rest_calls = module_invoke_all('mediamosa_register_rest_call');
  $rest_calls_doc = module_invoke_all('mediamosa_register_rest_call_doc');
  $rest_calls = array_merge_recursive($rest_calls, $rest_calls_doc);

  return $rest_calls;
}

/**
 * The main REST call browse form.
 */
function mediamosa_maintenance_browse_restcall_list_form() {

  $item_count_total = 0;

  // Get the filters.
  $filters = _mediamosa_maintenance_browse_restcalls_filters();

  if (empty($_SESSION['mediamosa_restcalls_filter'])) {
    $_SESSION['mediamosa_restcalls_filter'] = _mediamosa_maintenance_browse_restcalls_filter_default();
  }

  // Get the REST calls.
  $rest_calls = _mediamosa_maintenance_browse_restcalls_get();

  // Skip internal calls, but count the rest.
  foreach ($rest_calls as $uri => $method_rest_call) {
    foreach ($method_rest_call as $method => $rest_call) {

      // Set the method here.
      $rest_call[mediamosa_rest_call::METHOD] = $method;
      $rest_call = mediamosa_rest::set_default_rest_call($rest_call);

      // Don't show internals.
      if (isset($rest_call['access']) && ($rest_call[mediamosa_rest_call::ACCESS] == mediamosa_rest_call::ACCESS_INTERNAL_ONLY)) {
        unset($rest_calls[$uri][$method]);
        continue;
      }

      foreach ($filters as $name => $filter) {
        if (is_array($rest_call[$name])) {
          foreach ($rest_call[$name] as $value) {
            if (!in_array($value, $_SESSION['mediamosa_restcalls_filter'][$name])) {
              unset($rest_calls[$uri][$method]);
              continue 3;
            }
          }
        }
        elseif (!in_array($rest_call[$name], $_SESSION['mediamosa_restcalls_filter'][$name])) {
          unset($rest_calls[$uri][$method]);
          continue 2;
        }
      }

      // Status filter.

      // Uri entered?
      if (!empty($_SESSION['mediamosa_restcalls_filter']['uri'])) {
        if (stripos($uri, $_SESSION['mediamosa_restcalls_filter']['uri']) === FALSE) {
          unset($rest_calls[$uri][$method]);
          continue;
        }
      }

      // Ok, added.
      $item_count_total++;
    }
  }

  // Sort the rest calls on URI.
  ksort($rest_calls);

  $item_limit = mediamosa_settings::maintenance_items_per_page();
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $offset = $page * $item_limit;

  $header = array(
    array('data' => t('URI')),
    array('data' => t('REST Interfaces'), 'class' => 'header-interface'),
    array('data' => t('Method'), 'class' => 'header-method'),
    array('data' => t('Status'), 'class' => 'header-status'),
    array('data' => t('Version'), 'class' => 'header-version'),
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

      // Set by hand for set_default_rest_call().
      $rest_call[mediamosa_rest_call::METHOD] = $method;

      // Setup default values.
      $rest_call = mediamosa_rest::set_default_rest_call($rest_call);

      // Next position.
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

      $for_interface = array();
      foreach ($rest_call[mediamosa_rest_call::FOR_INTERFACE] as $interface) {
        $for_interface[] = $filters[mediamosa_rest_call::FOR_INTERFACE]['options'][$interface];
      }

      $rows[] = array(
        l(t('/' . $uri), 'admin/mediamosa/browse/restcall/' . str_replace('/', '-', $uri) . '/' . urlencode($method)) . ' ' . implode(' ', $warnings),
        array('data' => implode(',', $for_interface), 'class' => 'header-interface'),
        array('data' => $method, 'class' => 'header-method'),
        array('data' => $rest_call[mediamosa_rest_call::STATUS], 'class' => 'header-status'),
        array('data' => $rest_call[mediamosa_rest_call::VERSION], 'class' => 'header-version'),
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

      // Possible aliases.
      $aliases = !empty($item[mediamosa_rest_call::VAR_ALIAS_FOR]) ? t("This variable also works when specified as (depricated) alias; @aliases", array('@aliases' => implode(', ', $item[mediamosa_rest_call::VAR_ALIAS_FOR]))) : '';

      $rows[] = array(
        $param . $is_array,
        mediamosa_sdk::typeAlias2Type($item[mediamosa_rest_call::VAR_TYPE]),
        (isset($item[mediamosa_rest_call::VAR_DESCRIPTION]) ? $item[mediamosa_rest_call::VAR_DESCRIPTION] : '') . (!empty($allowed_values) ? '<br /><small>Value must be either "' . implode('" or "', $allowed_values) . '".' : '') . (!empty($aliases) ? '<br /><small>' . $aliases . '</small>' : ''),
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

      // Possible aliases.
      $aliases = !empty($item[mediamosa_rest_call::VAR_ALIAS_FOR]) ? t("This variable also works when specified as (depricated) alias; @aliases", array('@aliases' => implode(', ', $item[mediamosa_rest_call::VAR_ALIAS_FOR]))) : '';

      $rows[] = array(
        $param . $is_array,
        mediamosa_sdk::typeAlias2Type($item[mediamosa_rest_call::VAR_TYPE]),
        (isset($item[mediamosa_rest_call::VAR_DESCRIPTION]) ? $item[mediamosa_rest_call::VAR_DESCRIPTION] : '') . (!empty($allowed_values) ? '<br /><small>Value (when supplied) must be either "' . implode('" or "', $allowed_values) . '".' : '') . (!empty($aliases) ? '<br /><small>' . $aliases . '</small>' : ''),
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
