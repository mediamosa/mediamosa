<?php
/**
 * @file
 * Browser of REST calls.
 */

/**
 * The main REST call browse page.
 */
function _mediamosa_maintenance_browse_restcall_page() {
  // Add our CSS.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance_browse') . '/mediamosa_maintenance_browse_restcalls.css');

  return drupal_get_form('mediamosa_maintenance_browse_restcall_filter_form');
}

/**
 * Generate the filters for the listing.
 *
 * @return array
 *   A description of the filters.
 */
function _mediamosa_maintenance_browse_restcalls_filters() {

  // Collect the versions.
  $versions = array();

  // Need restcalls for versions.
  $rest_calls = _mediamosa_maintenance_browse_restcalls_get();

  // Collect versions.
  foreach ($rest_calls as $method_rest_call) {
    foreach ($method_rest_call as $rest_call) {
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
        mediamosa_rest_call::METHOD_OPTIONS => t('OPTIONS'),
      ),
      'default' => array(
        mediamosa_rest_call::METHOD_DELETE => mediamosa_rest_call::METHOD_DELETE,
        mediamosa_rest_call::METHOD_GET => mediamosa_rest_call::METHOD_GET,
        mediamosa_rest_call::METHOD_POST => mediamosa_rest_call::METHOD_POST,
        mediamosa_rest_call::METHOD_PUT => mediamosa_rest_call::METHOD_PUT,
        mediamosa_rest_call::METHOD_OPTIONS => mediamosa_rest_call::METHOD_OPTIONS,
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
    mediamosa_rest_call::ACCESS => array(
      'title' => t('Access'),
      'options' => array(
        mediamosa_rest_call::ACCESS_AUTHENTICATED => t('Normal authentication required'),
        mediamosa_rest_call::ACCESS_EXTERNAL => t('No authentication allowed'),
        mediamosa_rest_call::ACCESS_INTERNAL => t('Internal REST call'),
      ),
      'default' => array(
        mediamosa_rest_call::ACCESS_AUTHENTICATED => mediamosa_rest_call::ACCESS_AUTHENTICATED,
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
 * @see mediamosa_maintenance_browse_restcall_filter_form_submit()
 * @see mediamosa_maintenance_browse_restcall_filter_form_validate()
 */
function mediamosa_maintenance_browse_restcall_filter_form($form) {

  // Get the filters.
  $filters = _mediamosa_maintenance_browse_restcalls_filters();

  $form['listing'] = _mediamosa_maintenance_browse_restcall_list();

  // Show as settings form look.
  $form['#theme'] = 'system_settings_form';
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['#mediamosa_no_clone'] = TRUE;

  $form['actions']['restcall-filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter REST calls'),
    '#collapsible' => TRUE,
  );

  foreach ($filters as $key => $filter) {
    $form['actions']['restcall-filters'][$key] = array(
      '#title' => $filter['title'],
      '#type' => 'select',
      '#multiple' => TRUE,
      '#size' => 6,
      '#options' => $filter['options'],
      '#default_value' => empty($_SESSION['mediamosa_restcalls_filter'][$key]) ? $filter['default'] : $_SESSION['mediamosa_restcalls_filter'][$key],
    );
  }

  $form['actions']['restcall-filters']['uri'] = array(
    '#type' => 'textfield',
    '#title' => t('URI'),
    '#description' => t('Enter part of uri to filter.'),
    '#size' => 40,
    '#maxlength' => 100,
    '#required' => FALSE,
    '#default_value' => empty($_SESSION['mediamosa_restcalls_filter']['uri']) ? '' : $_SESSION['mediamosa_restcalls_filter']['uri'],
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
  );

  if (!empty($_SESSION['mediamosa_restcalls_filter']) && $_SESSION['mediamosa_restcalls_filter'] != _mediamosa_maintenance_browse_restcalls_filter_default()) {
    $form['actions']['reset'] = array(
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
  if ($form_state['values']['op'] == t('Filter') && empty($form_state['values']['status'])) {
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

      // Unset page when we press filter, so our results start at page 0.
      if ($_SESSION['mediamosa_restcalls_filter'] != _mediamosa_maintenance_browse_restcalls_filter_default()) {
        unset($_GET['page']);
      }
      break;

    case t('Reset'):
      $_SESSION['mediamosa_restcalls_filter'] = _mediamosa_maintenance_browse_restcalls_filter_default();
      unset($_GET['page']);
      break;
  }
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
function _mediamosa_maintenance_browse_restcall_list() {

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

      foreach (array_keys($filters) as $name) {
        // Filter on selected access.
        if ($name == mediamosa_rest_call::ACCESS) {
          $values_total = 0;
          foreach ($_SESSION['mediamosa_restcalls_filter'][$name] as $value) {
            $values_total |= $value;
          }

          // Exclusive internal only?
          if ($values_total == mediamosa_rest_call::ACCESS_INTERNAL_ONLY) {
            if ($rest_call[mediamosa_rest_call::ACCESS] != mediamosa_rest_call::ACCESS_INTERNAL_ONLY) {
              unset($rest_calls[$uri][$method]);
              continue 2;
            }
          }
          else {
            // Remove the found filter bits.
            $access = $rest_call[$name] & ~$values_total;

            // Not part of the access?
            if ($access == $rest_call[$name]) {
              unset($rest_calls[$uri][$method]);
              continue 2;
            }
          }
        }
        else {
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
  $page = isset($_GET['page']) ? (int) $_GET['page'] : 0;
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
        $warnings[] = '<small><sup>1</sup></small>';
      }
      if (!$has_example_response) {
        $has_warnings = TRUE;
        $warnings[] = '<small><sup>2</sup></small>';
      }

      $for_interface = array();
      foreach ($rest_call[mediamosa_rest_call::FOR_INTERFACE] as $interface) {
        $for_interface[] = $filters[mediamosa_rest_call::FOR_INTERFACE]['options'][$interface];
      }

      $rows[] = array(
        l(t('/' . $uri), 'admin/mediamosa/tools/restcall/' . str_replace('/', '-', $uri) . '/' . urlencode($method)) . ' ' . implode(' ', $warnings),
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
  $rest_call = mediamosa_rest::set_default_rest_call($rest_calls[$uri][$method]);

  // Create class from it.
  $mediamosa_rest_call = new $rest_calls[$uri][$method][mediamosa_rest_call::CLASS_NAME]($rest_call);
  $mediamosa_rest_call->{mediamosa_rest_call::CLASS_NAME} = $rest_calls[$uri][$method][mediamosa_rest_call::CLASS_NAME];

  // Check if override for class selection is here.
  if (method_exists($mediamosa_rest_call, 'get_object_rest_call')) {
    $mediamosa_rest_call = $mediamosa_rest_call->get_object_rest_call($rest_call, isset($rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES]) ? $rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES] : array());
  }

  // Set as uri values, so we dont fail checks.
  if (isset($rest_calls[$uri][$method][mediamosa_rest_call::DEFAULT_PARAMS_VALUES])) {
    foreach ($rest_calls[$uri][$method][mediamosa_rest_call::DEFAULT_PARAMS_VALUES] as $param => $value) {
      $mediamosa_rest_call->{mediamosa_rest_call::URI_PARAMS}[$param] = $value;
    }
  }

  // Turn off so we can call the get_var_setup without problems.
  // Although get_var_setup should not test variables.
  $mediamosa_rest_call->set_check_for_unspecified(FALSE);

  // Get the var setup for this call.
  $var_setup = $mediamosa_rest_call->get_var_setup();

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

  if ($mediamosa_rest_call->access >= mediamosa_rest_call::ACCESS_AUTHENTICATED && ($mediamosa_rest_call->access & mediamosa_rest_call::ACCESS_EXTERNAL) != mediamosa_rest_call::ACCESS_EXTERNAL) {
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
    '#rest_call' => $mediamosa_rest_call,
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
  $mediamosa_rest_call = $variables['rest_call'];
  $var_setup = $variables['var_setup'];

  // Collect the authorization.
  $request_authorization = array();

  // Build up the text explaning the access.
  if ($mediamosa_rest_call->access == mediamosa_rest_call::ACCESS_AUTHENTICATED_ONLY) {
    $request_authorization[] = t('needs EGA authentication.');
  }
  elseif ($mediamosa_rest_call->access == mediamosa_rest_call::ACCESS_EXTERNAL_ONLY) {
    $request_authorization[] = t('does not need any authentication.');
  }
  elseif ($mediamosa_rest_call->access == mediamosa_rest_call::ACCESS_INTERNAL_ONLY) {
    $request_authorization[] = t('can only be access by internal access.');
  }
  else {
    if ($mediamosa_rest_call->access & mediamosa_rest_call::ACCESS_AUTHENTICATED) {
      $request_authorization[] =  t('can be accessed by EGA authentication');
    }
    if ($mediamosa_rest_call->access & mediamosa_rest_call::ACCESS_EXTERNAL) {
      $request_authorization[] =  t('can be accessed without authentication');
    }
    if ($mediamosa_rest_call->access & mediamosa_rest_call::ACCESS_INTERNAL) {
      $request_authorization[] =  t('can be accessed by internal authentication');
    }
  }

  if (count($request_authorization) == 1) {
    $request_authorization = reset($request_authorization);
  }
  else {
    if (count($request_authorization) == 2) {
      $request_authorization = implode(' or ', $request_authorization);
    }
    else {
      $last_one = array_pop($request_authorization);
      $request_authorization = implode(', ', $request_authorization) . ' or ' . $last_one;
    }
  }

  // Set request text.
  $variables['rest_call']->request_authorization = t('This REST call @access.', array('@access' => $request_authorization));

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
  $rows = array();
  $rows_internal = array();

  if (!isset($var_setup[mediamosa_rest_call::VARS])) {
    $var_setup[mediamosa_rest_call::VARS] = array();
  }

  // First required.
  foreach ($var_setup[mediamosa_rest_call::VARS] as $param => $item) {
    if (isset($item[mediamosa_rest_call::VAR_IS_REQUIRED]) && $item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_YES) {
      if (isset($item[mediamosa_rest_call::VAR_IS_INTERNAL_ONLY]) && $item[mediamosa_rest_call::VAR_IS_INTERNAL_ONLY] == mediamosa_rest_call::VAR_IS_INTERNAL_ONLY_YES) {
        continue;
      }

      // Add item.
      _template_preprocess_restcall_doc_item_helper($param, $item, $rows, TRUE);
    }
  }

  // Not required.
  foreach ($var_setup[mediamosa_rest_call::VARS] as $param => $item) {
    if (!isset($item[mediamosa_rest_call::VAR_IS_REQUIRED]) || $item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_NO) {
      if (isset($item[mediamosa_rest_call::VAR_IS_INTERNAL_ONLY]) && $item[mediamosa_rest_call::VAR_IS_INTERNAL_ONLY] == mediamosa_rest_call::VAR_IS_INTERNAL_ONLY_YES) {
        continue;
      }

      $is_array = isset($item[mediamosa_rest_call::VAR_IS_ARRAY]) && $item[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES ? '[]' : '';

      // Add item.
      _template_preprocess_restcall_doc_item_helper($param, $item, $rows, FALSE);
    }
  }

  // Internals required.
  foreach ($var_setup[mediamosa_rest_call::VARS] as $param => $item) {
    if (isset($item[mediamosa_rest_call::VAR_IS_REQUIRED]) && $item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_YES) {
      if (!isset($item[mediamosa_rest_call::VAR_IS_INTERNAL_ONLY]) || $item[mediamosa_rest_call::VAR_IS_INTERNAL_ONLY] != mediamosa_rest_call::VAR_IS_INTERNAL_ONLY_YES) {
        continue;
      }

      $is_array = isset($item[mediamosa_rest_call::VAR_IS_ARRAY]) && $item[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES ? '[]' : '';

      // Add item.
      _template_preprocess_restcall_doc_item_helper($param, $item, $rows_internal, TRUE);
    }
  }

  // Internals not required.
  foreach ($var_setup[mediamosa_rest_call::VARS] as $param => $item) {
    if (!isset($item[mediamosa_rest_call::VAR_IS_REQUIRED]) || $item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_NO) {
      if (!isset($item[mediamosa_rest_call::VAR_IS_INTERNAL_ONLY]) || $item[mediamosa_rest_call::VAR_IS_INTERNAL_ONLY] != mediamosa_rest_call::VAR_IS_INTERNAL_ONLY_YES) {
        continue;
      }

      // Add item.
      _template_preprocess_restcall_doc_item_helper($param, $item, $rows_internal, FALSE);
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

  if (!empty($rows_internal)) {
    // Parameter list.
    $variables['parameters_internal'] = theme(
      'table',
      array(
        'header' => $header,
        'rows' => $rows_internal,
        'empty' => t('This REST call has no input parameters.'),
      )
    );
  }

  // Response fields.
  $header = array(
    array('data' => t('Field')),
    array('data' => t('Description')),
  );

  if (!empty($mediamosa_rest_call->response_fields)) {
    $rows = array();
    foreach ($mediamosa_rest_call->response_fields as $key => $value) {
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
  elseif ($mediamosa_rest_call->response_fields === FALSE) {
    $variables['response_fields'] = t('This REST call does not have any response fields and will always return empty items array');
  }

  if (empty($variables['rest_call']->example_request)) {
    $variables['warnings'][] = t('No example request specified.');
  }
  if ($variables['rest_call']->example_response === FALSE) {
    if ($mediamosa_rest_call->{mediamosa_rest_call::METHOD} == mediamosa_rest_call::METHOD_GET) {
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

/**
 * Helper for adding rows.
 *
 * @param $param
 *   The param name.
 * @param $item
 *   The item to add.
 * @param $rows
 *   The result rows.
 * @param $required
 *   Is TRUE when required.
 */
function _template_preprocess_restcall_doc_item_helper($param, $item, &$rows, $required = TRUE) {
  $is_array = isset($item[mediamosa_rest_call::VAR_IS_ARRAY]) && $item[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES ? '[]' : '';

  // Get allowed values.
  $allowed_values = !empty($item[mediamosa_rest_call::VAR_ALLOWED_VALUES]) ? $item[mediamosa_rest_call::VAR_ALLOWED_VALUES] : array();

  // Possible aliases.
  $aliases = !empty($item[mediamosa_rest_call::VAR_ALIAS_FOR]) ? t("This variable also works when specified as (depricated) alias; @aliases", array('@aliases' => implode(', ', $item[mediamosa_rest_call::VAR_ALIAS_FOR]))) : '';

  $rows[] = array(
    $param . $is_array,
    mediamosa_sdk::typeAlias2Type($item[mediamosa_rest_call::VAR_TYPE]),
    (isset($item[mediamosa_rest_call::VAR_DESCRIPTION]) ? $item[mediamosa_rest_call::VAR_DESCRIPTION] : '') . (!empty($allowed_values) ? '<br /><small>Value (when supplied) must be either "' . implode('" or "', $allowed_values) . '".' : '') . (!empty($aliases) ? '<br /><small>' . $aliases . '</small>' : ''),
    $required ? t('Required') : t('Optional'),
    isset($item[mediamosa_rest_call::VAR_DEFAULT_VALUE]) ? $item[mediamosa_rest_call::VAR_DEFAULT_VALUE] : '',
    isset($item[mediamosa_rest_call::VAR_RANGE_END]) ? $item[mediamosa_rest_call::VAR_RANGE_END] : '-',
  );
}
