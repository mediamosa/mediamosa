<?php
/**
 * @file
 * Swagger class file.
 */

/**
 * The REST class object contains the REST layer.
 *
 * Translates Uri into rest call functions and handles all out going data and
 * errors.
 */
class mediamosa_swagger extends mediamosa_rest {
  /**
   * Get a list of REST calls registered with our Drupal hook.
   *
   * @param int $limit
   *   Maximum of items in result.
   * @param int $offset
   *   Offset in items list.
   * @param array $options
   *   - Key ('method')
   *    The name of the first array key to use.
   *   - Key2 ('Uri')
   *    The name of the second array key to use (keep empty to skip).
   *   - get_class_name (FALSE)
   *    Get the class filename stored in the registry for REST class.
   *   - restrict_interface (TRUE)
   *    Test if the interface matches the interface of the REST call. E.g. test
   *    if an upload REST call is called from the upload interface, else skip.
   */
  public static function get_rest_calls_swagger($limit = 0, $offset = 0, array $options = array()) {

    $options += array(
      'key' => mediamosa_rest_call::METHOD,
      'key2' => mediamosa_rest_call::URI,
      'get_class_name' => FALSE,
      'restrict_interface' => TRUE,
    );

    // Find all REST calls of enabled modules.
    $rest_calls = module_invoke_all('mediamosa_register_rest_call');
    $rest_calls_doc = module_invoke_all('mediamosa_register_rest_call_doc');
    $rest_calls = array_merge_recursive($rest_calls, $rest_calls_doc);

    // Sort on URI.
    ksort($rest_calls);

    // Start with an empty result.
    $results = array();

    if (!$limit) {
      $limit = -1;
    }

    $pos = 0;
    foreach ($rest_calls as $uri => $rest_call_2) {
      foreach ($rest_call_2 as $method => $rest_call) {
        // Set method.
        $rest_call[mediamosa_rest_call::METHOD] = $method;

        // Setup default.
        $rest_call = self::set_default_rest_call($rest_call);

        // Don't show internal restcalls.
        if ($rest_call['access'] <= 1) {
          continue;
        }

        $rest_call[mediamosa_rest_call::URI] = $uri;
        $rest_call[mediamosa_rest_call::ID] = $pos++;

        // Sort on Property.
        ksort($rest_call);

        $results[$uri][$method] = $rest_call;

        // No more?
        if (!$limit) {
          break;
        }

        if ($limit) {
          $limit--;
        }
      }

      if (!$limit) {
        break;
      }
    }
    return $results;
  }

  /**
   * Convert MediaMosa type to simple types.
   *
   * Simple types are:"string", "number", "integer", "boolean", "array", "file".
   *
   * @param string $type
   *   Name of type.
   *
   * @return string
   *   The simple type definition.
   */
  public static function type_to_simple_type($type) {
    switch ($type) {
      case mediamosa_sdk::TYPE_LANGUAGE_CODE_ISO_639_3:
      case mediamosa_sdk::TYPE_LANGUAGE_CODE:
      case mediamosa_sdk::TYPE_JOB:
      case mediamosa_sdk::TYPE_JOB_STATUS:
      case mediamosa_sdk::TYPE_COMMAND:
      case mediamosa_sdk::TYPE_ALPHA:
      case mediamosa_sdk::TYPE_MEDIAFILE_ID_PREFIX:
      case mediamosa_sdk::TYPE_TICKET_ID_PREFIX:
      case mediamosa_sdk::TYPE_TICKET_ID:
      case mediamosa_sdk::TYPE_MEDIAFILE_ID:
      case mediamosa_sdk::TYPE_MEDIAMOSA_VERSION:
      case mediamosa_sdk::TYPE_ALPHA_NUM:
      case mediamosa_sdk::TYPE_USER_ID:
      case mediamosa_sdk::TYPE_GROUP_ID:
      case mediamosa_sdk::TYPE_ASSET_ID:
      case mediamosa_sdk::TYPE_COLLECTION_ID:
      case mediamosa_sdk::TYPE_BATCH_ID:
      case mediamosa_sdk::TYPE_PRINTABLE:
      case mediamosa_sdk::TYPE_ALPHA_NUM_UNDERSCORE_TAG:
      case mediamosa_sdk::TYPE_RESPONSE_TYPE:
      case mediamosa_sdk::TYPE_ALPHA_NUM_UNDERSCORE:
      case mediamosa_sdk::TYPE_CQL_ASSET:
      case mediamosa_sdk::TYPE_CQL_COLLECTION:
      case mediamosa_sdk::TYPE_CQL_JOB:
      case mediamosa_sdk::TYPE_OAUTH_SIGNATURE:
      case mediamosa_sdk::TYPE_OAUTH_SIGNATURE_METHOD:
      case mediamosa_sdk::TYPE_OAUTH_VERSION:
      case mediamosa_sdk::TYPE_OAUTH_TOKEN:
      case mediamosa_sdk::TYPE_OAUTH_VERIFIER:
      case mediamosa_sdk::TYPE_STRING:
      case mediamosa_sdk::TYPE_XML_OAI:
      case mediamosa_sdk::TYPE_XML_VALIDATED:
      case mediamosa_sdk::TYPE_SEARCH_MATCH:
      case mediamosa_sdk::TYPE_SEARCH_STRING:
      case mediamosa_sdk::TYPE_SEARCH_INT:
      case mediamosa_sdk::TYPE_SEARCH_DATETIME:
      case mediamosa_sdk::TYPE_SEARCH_BOOL:
      case mediamosa_sdk::TYPE_MIMETYPE:
      case mediamosa_sdk::TYPE_DATETIME:
      case mediamosa_sdk::TYPE_DATETIME_UTC:
      case mediamosa_sdk::TYPE_ORDER_DIRECTION:
      case mediamosa_sdk::TYPE_URI:
      case mediamosa_sdk::TYPE_URL:
      case mediamosa_sdk::TYPE_URL_URI:
      case mediamosa_sdk::TYPE_ACL_GROUP_TYPE:
      case mediamosa_sdk::TYPE_DELETE:
      case mediamosa_sdk::TYPE_DOMAIN:
      case mediamosa_sdk::TYPE_REALM:
      case mediamosa_sdk::TYPE_OPERATOR:
      case mediamosa_sdk::TYPE_FILENAME:
        return 'string';

      case mediamosa_sdk::TYPE_JOB_PROGRESS:
      case mediamosa_sdk::TYPE_FLOAT:
        return 'number';

      case mediamosa_sdk::TYPE_APP_ID:
      case mediamosa_sdk::TYPE_SERIAL:
      case mediamosa_sdk::TYPE_LIMIT:
      case mediamosa_sdk::TYPE_JOB_ID:
      case mediamosa_sdk::TYPE_UINT:
      case mediamosa_sdk::TYPE_EOR:
      case mediamosa_sdk::TYPE_INT:
      case mediamosa_sdk::TYPE_SUPPLEMENT_ID:
        return 'integer';

      case mediamosa_sdk::TYPE_BOOL_IS_SLAVED:
      case mediamosa_sdk::TYPE_BOOL:
        return 'boolean';

      case mediamosa_sdk::TYPE_FILE:
        return 'file';
    }
  }

}
