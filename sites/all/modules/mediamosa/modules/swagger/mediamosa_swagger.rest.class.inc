<?php
/**
 * @file
 * Swagger rest calls.
 */

/**
 * Uri: /doc/swagger.json [GET].
 *
 * Rest calls for/of rest interface.
 */
class mediamosa_rest_call_doc_swagger extends mediamosa_rest_call {

  const PRETTY_PRINT = 'pretty_print';

  /**
   * Implements get_var_setup().
   */
  public function get_var_setup() {

    $var_setup = array(
      self::VARS => array(
        self::PRETTY_PRINT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Pretty Print.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  /**
   * Implements do_call().
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    // note: swagger does not support an limit/offset.
    $rest_calls = mediamosa_swagger::get_rest_calls_swagger(0, 0);
    foreach ($rest_calls as $uri => $rest_call) {
      foreach ($rest_call as $method => $call) {

        $rest_call_rest = mediamosa_swagger::get_uri_definition($uri, $method);

        if (!$rest_call_rest) {
          continue;
        }

        // Skip deprecated.
        if ($rest_call_rest[mediamosa_rest_call::STATUS] == mediamosa_rest_call::STATUS_DEPRECATED) {
          continue;
        }

        // Create class from it.
        $the_rest_call = new $rest_call_rest[mediamosa_rest_call::CLASS_NAME]($rest_call_rest);

        // Check if override for class selection is here.
        if (method_exists($the_rest_call, 'get_object_rest_call')) {
          $the_rest_call = $the_rest_call->get_object_rest_call($rest_call);
        }

        // Turn off so we don't fail.
        $the_rest_call->set_check_for_unspecified(FALSE);

        // Get the var setup.
        $var_setup = $the_rest_call->get_var_setup();

        $rest_call[$method]['parameters'] = $var_setup['vars'];

        $mediamosa->add_item($rest_call);
      }
    }
  }

  /**
   * Generate the JSON output.
   *
   * @return string
   *   Swagger json output.
   */
  public function generate_json(&$response) {

    header('Access-Control-Allow-Origin: *');

    $swagger_version = '2.0';
    $info = array(
      'version' => '1.0.2',
      'title' => 'Swagger API Mediamosa',
      'description' => 'API documentation of the MediaMosa API, see mediamosa.org for more information.',
    );

    global $base_url;
    $swagger_base = variable_get('mediamosa_swagger_base_url', $base_url);
    $url = parse_url($swagger_base);
    $schemes = array($url['scheme']);

    $consumes = array('application/xml');
    $produces = array('application/xml');

    $response['data']['swagger'] = $swagger_version;
    $response['data']['info'] = $info;
    $response['data']['host'] = $url['host'];
    $response['data']['basePath'] = $url['path'];
    $response['data']['consumes'] = $consumes;
    $response['data']['schemes'] = $schemes;
    $response['data']['produces'] = $produces;

    $pos = 0;
    $tag_list = array();
    foreach ($response['items'] as $rest_call) {

      if (($rest_call['GET']['uri'] == 'simpletest') ||
        (strpos($rest_call['GET']['uri'], 'openapi') !== FALSE) ||
        (strpos($rest_call['POST']['uri'], 'storage') !== FALSE) ||
        (strpos($rest_call['OPTIONS']['uri'], 'options') !== FALSE) ||
        ($rest_call['GET']['uri'] == 'status')) {
        continue;
      }
      foreach ($rest_call as $key => $call) {

        $call['uri'] = '/' . $call['uri'];
        // Replace any $param pattern with {param}.
        $call['uri'] = preg_replace('/\\$(\\w*)/', '{$1}', $call['uri']);

        $call['method'] = strtolower($call['method']);

        $response['data']['paths'][$call['uri']][$call['method']]['summary'] = $call['title'];
        $response['data']['paths'][$call['uri']][$call['method']]['description'] = $call['description'];

        $tag = explode('/', $call['uri']);
        $tag = $tag[1];
        $response['data']['paths'][$call['uri']][$call['method']]['tags'][0] = $tag;
        array_push($tag_list, $tag);

        $i = 0;
        foreach ($call['parameters'] as $key => $parameter) {
          if (($parameter['internal_only'] != 'YES') &&
            ($parameter['description'] != 'NOT USED.') &&
            ($parameter[mediamosa_rest_call::VAR_IS_READ_ONLY] != mediamosa_rest_call::VAR_IS_READ_ONLY_YES) &&
            (strpos($parameter['description'], 'use CQL instead') === FALSE) &&
            ($key != 'mediamosa_version')) {
            $response['data']['paths'][$call['uri']][$call['method']]['parameters'][$i]['name'] = $key;
            $response['data']['paths'][$call['uri']][$call['method']]['parameters'][$i]['description'] = $parameter['description'];
            $response['data']['paths'][$call['uri']][$call['method']]['parameters'][$i]['type'] = mediamosa_swagger::type_to_simple_type($parameter['type']);
            if ($parameter['type'] == 'FILE') {
              $response['data']['paths'][$call['uri']][$call['method']]['parameters'][$i]['consumes'] = array('multipart/form-data', 'application/x-www-form-urlencoded');
            }
            $response['data']['paths'][$call['uri']][$call['method']]['parameters'][$i]['format'] = $parameter['type'];

            // 'in': The location of the parameter. Possible values are "query",
            // "header", "path", "formData" or "body".
            if (strpos($call['uri'], '{' . $key . '}') !== FALSE) {
              $response['data']['paths'][$call['uri']][$call['method']]['parameters'][$i]['in'] = 'path';
            }
            elseif (($call['method'] == 'post') || ($call['method'] == 'put')) {
              $response['data']['paths'][$call['uri']][$call['method']]['parameters'][$i]['in'] = 'formData';
            }
            else {
              $response['data']['paths'][$call['uri']][$call['method']]['parameters'][$i]['in'] = 'query';
            }
            $response['data']['paths'][$call['uri']][$call['method']]['parameters'][$i]['required'] = isset($parameter['is_required']) && $parameter['is_required'] == 'YES' ? TRUE : FALSE;
            $i++;
          }
        }

        $response['data']['paths'][$call['uri']][$call['method']]['responses']['200']['schema']['$ref'] = '#/definitions/item' . $pos;
        $response['data']['paths'][$call['uri']][$call['method']]['responses']['200']['description'] = 'Succes';
        $response['data']['definitions']['item' . $pos]['type'] = 'object';

        foreach ($call['response_fields'] as $x => $response_field) {
          $response['data']['definitions']['item' . $pos]['properties'][$x]['type'] = $response_field;
          $response['data']['definitions']['item' . $pos]['properties'][$x]['description'] = $response_field;
        }

        $pos++;
      }
    }

    foreach (array_unique($tag_list) as $value) {
      $response['data']['tags'][]['name'] = $value;
    }

    $pretty_print = $this->get_param_value(self::PRETTY_PRINT);
    if ($pretty_print) {
      return json_encode($response['data'], JSON_PRETTY_PRINT | JSON_HEX_TAG | JSON_HEX_APOS | JSON_HEX_AMP | JSON_HEX_QUOT);
    }
    return drupal_json_encode($response['data']);
  }

}
