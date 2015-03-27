<?php
/**
 * @file
 * Our extended classes for Solr connections.
 */

// We need SolrPhpClient class.
if (!class_exists('Apache_Solr_Service')) {
  // If we haven't install properly, the requirements hook will tell us.
  // So ignore if we can't find it here.
  // Installing the mediamosa_solr module will fail if during install the class
  // isn't found either. So don't worry about the problem we are not loading
  // MediamosaApacheSolrService for registry indexing (drupal autoloader).
  if (!file_exists(dirname(__FILE__) . '/3rdparty/SolrPhpClient/Apache/Solr/Service.php')) {
    return;
  }

  include_once dirname(__FILE__) . '/3rdparty/SolrPhpClient/Apache/Solr/Service.php';
}

assert(class_exists('Apache_Solr_Service'));

/**
 * We will extend on the Apache Solr Service just for future enhancements.
 *
 * All specific MediaMosa functions must start with mediamosa (mediamosaSolr..)
 * to prevent double usage on lower class names in future.
 */
class mediamosa_solr_apache_solr_service extends Apache_Solr_Service {

  // ------------------------------------------------------------------- Consts.
  const MEDIAMOSA_SOLR_DEFAULT_HOST = 'solr.mediamosa.local';
  const MEDIAMOSA_SOLR_DEFAULT_PORT = 8983;
  const MEDIAMOSA_SOLR_DEFAULT_PATH = '/solr/';
  const MEDIAMOSA_SOLR_DEFAULT_SCHEME = 'http';

  const MLT_SERVLET = 'mlt';

  // --------------------------------------------------------------------- Vars.
  /**
   * The scheme ('http', 'https').
   *
   * @var string
   */
  protected $_scheme;

  /**
   * Constructed servlet full path URLs
   *
   * @var string
   */
  protected $_schemaNameUrl;

  // -------------------------------------------------------------- Constructor.
  /**
   * Constructor.
   */
  public function __construct($url = NULL) {

    if (!isset($url)) {
      $url = self::mediamosaGetUrl();
    }

    // Split up into scheme, host, port and path.
    $this->_scheme = self::mediamosaGetScheme();
    $host = self::mediamosaGetHost($url);
    $port = self::mediamosaGetPort($url);
    $path = self::mediamosaGetPath($url);

    parent::__construct($host, $port, $path);
  }

  /**
   * Construct the Full URLs for the three servlets we reference
   */
  protected function _initUrls() {
    parent::_initUrls();

    $this->_schemaNameUrl = $this->_constructUrl('schema/name');
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Filter out special delimeters ^~.
   *
   * We use ^ and ~ for delimeters, we need to filter them out for content
   * (sometimes).
   */
  public static function filterDelimeters($value) {
    return str_replace(array('_'), '', $value);
  }

  /**
   * Escape a string value and escape all special chars, including wildcards.
   *
   * Use this function when you concat with wildcard, but the original string
   * may not have active wildcards.
   *
   * e.g.;
   * escapeForSpace('foo*') . '*';
   * Will search for items that start with foo* (foo with *).
   *
   * @param string $value
   *   The value to escape.
   *
   * @return string
   *   The escaped string.
   */
  public static function escapeForSpace($value) {
    $pattern = '/(\+|\s|-|&&|\|\||!|\(|\)|\{|}|\[|]|\^|"|~|\*|\?|:|\\\)/';
    $replace = '\\\$1';

    return preg_replace($pattern, $replace, $value);
  }

  /**
   * Escape a string value.
   *
   * Escaping a value when using wildcards. When value is an integer or does not
   * contain wildcards, then use phrase() instead. Will escape spaces.
   *
   * @param string $value
   *   The value to escape.
   *
   * @return string
   *   The escaped string.
   */
  public static function escapeForWildcards($value) {
    $pattern = '/(\+|\s|-|&&|\|\||!|\(|\)|\{|}|\[|]|\^|"|~|:|\\\)/';
    $replace = '\\\$1';

    // The preg_replace() will need to be converted for wildcards that are
    // allowed.
    return str_replace(array('\\*', '\\?'), array('*', '?'), preg_replace($pattern, $replace, $value));
  }

  /**
   * Return the url for the solr servlet.
   */
  public static function mediamosaGetUrl() {
    $url = variable_get('mediamosa_solr_url', self::MEDIAMOSA_SOLR_DEFAULT_SCHEME . '://' . self::MEDIAMOSA_SOLR_DEFAULT_HOST . '/');

    // Parse the url into parts.
    $parse_url = parse_url($url);

    // Path is mandatory.
    if (isset($parse_url['path']) && $parse_url['path'] == '/') {
      unset($parse_url['path']);
    }

    // Add possible missing stuff.
    $parse_url += array(
      'scheme'  => self::MEDIAMOSA_SOLR_DEFAULT_SCHEME,
      'host'  => self::MEDIAMOSA_SOLR_DEFAULT_HOST,
      'port'  => self::MEDIAMOSA_SOLR_DEFAULT_PORT,
      'path'  => self::MEDIAMOSA_SOLR_DEFAULT_PATH,
    );

    // Solr only uses scheme, host, port and path.
    return $parse_url['scheme'] . '://' . $parse_url['host'] . ':' . $parse_url['port'] . '/' . trim($parse_url['path'], '/') . '/';
  }

  /**
   * Sets the URL in Drupal variable.
   */
  public static function mediamosaSetUrl($url) {
    // Parse the url into parts.
    $parse_url = parse_url($url);

    // Path is mandatory.
    if (isset($parse_url['path']) && $parse_url['path'] == '/') {
      unset($parse_url['path']);
    }

    // Add possible missing stuff.
    $parse_url += array(
      'scheme'  => self::MEDIAMOSA_SOLR_DEFAULT_SCHEME,
      'host'  => self::MEDIAMOSA_SOLR_DEFAULT_HOST,
      'port'  => self::MEDIAMOSA_SOLR_DEFAULT_PORT,
      'path'  => self::MEDIAMOSA_SOLR_DEFAULT_PATH,
    );

    // Solr only uses scheme, host, port and path.
    variable_set('mediamosa_solr_url', $parse_url['scheme'] . '://' . $parse_url['host'] . ':' . $parse_url['port'] . '/' . trim($parse_url['path'], '/') . '/');
  }

  /**
   * Return the scheme (e.g. http[s]://) for the solr url.
   *
   * @param string $url
   *   (optional) The url of which to extract scheme.
   *
   * @return string
   *   The scheme.
   */
  public static function mediamosaGetScheme($url = NULL) {
    $url = isset($url) ? $url : self::mediamosaGetUrl();

    // Parse.
    return parse_url($url, PHP_URL_SCHEME);
  }

  /**
   * Return the host for the solr url.
   *
   * @param string $url
   *   (optional) The url of which to extract host.
   *
   * @return string
   *   The host.
   */
  public static function mediamosaGetHost($url = NULL) {
    $url = isset($url) ? $url : self::mediamosaGetUrl();

    // Parse.
    return parse_url($url, PHP_URL_HOST);
  }

  /**
   * Return the port for the solr url.
   *
   * @param string $url
   *   (optional) The url of which to extract port.
   *
   * @return int
   *   The port.
   */
  public static function mediamosaGetPort($url = NULL) {
    $url = isset($url) ? $url : self::mediamosaGetUrl();

    // Get me the port.
    $port = (int) parse_url($url, PHP_URL_PORT);

    // Port is optional in the URL.
    return $port > 0 ? (int) $port : self::MEDIAMOSA_SOLR_DEFAULT_PORT;
  }

  /**
   * Return the path for the solr path.
   *
   * @param string $url
   *   (optional) The url of which to extract path.
   *
   * @return string
   *   The path.
   */
  public static function mediamosaGetPath($url = NULL) {
    $url = isset($url) ? $url : self::mediamosaGetUrl();

    // Get me the path.
    $path = trim(parse_url($url, PHP_URL_PATH), '/');

    // Path must start and end with '/'.
    return !empty($path) ? '/' . $path . '/' : self::MEDIAMOSA_SOLR_DEFAULT_PATH;
  }

  /**
   * Get the Solr object.
   *
   * Will ping the first time if it is created the first time (when $ping is
   * TRUE).
   *
   * @param string $url
   *   (optional) The Solr server URL.
   * @param bool $ping
   *   Ping the server after creating the object.
   * @param bool $reset
   *   Reset the static storage of object.
   *
   * @return mediamosa_solr_apache_solr_service
   *   The created solr server object.
   *
   * @throws mediamosa_solr_exception
   */
  public static function mediamosaGetSolrObject($url = NULL, $ping = FALSE, $reset = FALSE) {
    $mediamosa_apache_solr_service = &drupal_static(__FUNCTION__);

    // Not provided? Get it.
    $url = !empty($url) ? $url : static::mediamosaGetUrl();

    // Put in static for reuse.
    if (empty($mediamosa_apache_solr_service[$url]) || $reset) {
      $mediamosa_apache_solr_service[$url] = new mediamosa_solr_apache_solr_service($url);

      if (empty($mediamosa_apache_solr_service[$url])) {
        throw new mediamosa_solr_exception('Unable to create Solr object');
      }

      // Ping the server.
      if ($ping) {
        // Ping it, will throw exception when failed.
        $pinged = $mediamosa_apache_solr_service[$url]->ping() !== FALSE;

        if (!$pinged) {
          throw new mediamosa_solr_exception('Unable to reach Apache Solr database');
        }
      }
    }

    // Now return the object.
    return $mediamosa_apache_solr_service[$url];
  }

  /**
   * Ping Solr with either given params or use default.
   *
   * @param string $url
   *   The server URL.
   * @param bool $fatal
   *   Set to TRUE when exeception needs to be thrown when failure.
   *
   * @return bool
   *   Will return TRUE for success, FALSE for failure. Will only return when
   *   $fatal is set to FALSE.
   *
   * @throws mediamosa_solr_exception (when $fatal == TRUE)
   */
  public static function solrServicePing($url = NULL, $fatal = FALSE) {
    // Get the solr connection and ping it.
    try {
      self::mediamosaGetSolrObject($url, TRUE);
    }
    catch (Exception $e) {
      if ($fatal) {
        throw $e;
      }

      return FALSE;
    }

    return TRUE;
  }

  /**
   * Retrieve the schema info.
   *
   * @return array|FALSE
   *   The schema info;
   *   'success': TRUE or FALSE indicating schema info was found.
   *   'name': The name of the schema, not available when success was FALSE.
   */
  public static function solrServiceGetSchemaInfo() {
    // Get the solr schema info.
    return self::mediamosaGetSolrObject()->getSchemaInfo();
  }

  // ------------------------------ Overruled on base class Apache_Solr_Service.
  /**
   * Return a valid http URL.
   *
   * Rebuild the URL using the given server's host, port and path and a
   * provided servlet name.
   *
   * @param string $servlet
   *   The path of the servlet, e.g. 'select', 'update' etc.
   * @param array $params
   *   The query parameter array to be processed, e.g. $_GET.
   *
   * @return string
   *   The constructed URL.
   */
  protected function _constructUrl($servlet, $params = array()) {

    $queryString = '';
    if (count($params)) {
      // Escape all parameters appropriately for inclusion in the query string.
      $escapedParams = array();

      foreach ($params as $key => $value) {
        $escapedParams[] = urlencode($key) . '=' . urlencode($value);
      }

      $queryString = $this->_queryDelimiter . implode($this->_queryStringDelimiter, $escapedParams);
    }

    return $this->_scheme . '://' . $this->_host . ':' . $this->_port . $this->_path . $servlet . $queryString;
  }

  /**
   * Simple Search interface
   *
   * @param string $query
   *   The raw query string.
   * @param int $offset
   *   The starting offset for result documents.
   * @param int $limit
   *   The maximum number of result documents to return.
   * @param array $params
   *   The key / value pairs for other query parameters (see Solr
   *   documentation), use arrays for parameter keys used more than once (e.g.
   *   facet.field).
   * @param string $method
   *   The HTTP method (Apache_Solr_Service::METHOD_GET or
   *   Apache_Solr_Service::METHOD::POST).
   *
   * @return Apache_Solr_Response
   *
   * @throws Apache_Solr_HttpTransportException
   *   If an error occurs during the service call.
   * @throws Apache_Solr_InvalidArgumentException
   *   If an invalid HTTP method is used.
   */
  public function search($query, $offset = 0, $limit = 10, $params = array(), $method = self::METHOD_GET) {
    $searchUrl = $this->_searchUrl;
    if (isset($params['mlt']) && mediamosa_unicode::strtolower($params['mlt']) === 'true') {
      $this->_searchUrl = $this->_constructUrl(self::MLT_SERVLET);
    }

    $result = parent::search($query, $offset, $limit, $params, $method);
    $this->_searchUrl = $searchUrl;
    return $result;
  }

  /**
   * Retrieve the schema info.
   *
   * @return array|FALSE
   *   The schema info;
   *   'success': TRUE or FALSE indicating schema info was found.
   *   'name': The name of the schema, not available when success was FALSE.
   */
  public function getSchemaInfo() {
    $schema_info = array(
      'success' => FALSE,
      'name' => 'Unable to retrieve schema information',
    );

    // Get the solr connection and ping it.
    try {
      $httpTransport = $this->getHttpTransport();
      $solrResponse = new Apache_Solr_Response($httpTransport->performGetRequest($this->_schemaNameUrl));
      if ($solrResponse->getHttpStatus() == 404) {
        throw new Apache_Solr_HttpTransportException($solrResponse);
      }
      $response = new mediamosa_solr_apache_solr_response($solrResponse);
      $schema_info['name'] = $response->getSchemaName();

      $schema_info['success'] = TRUE;
    }
    catch (Apache_Solr_HttpTransportException $e) {
      if ($e->getCode() == 404) {
        return FALSE;
      }

      $schema_info['name'] = $e->getMessage() . ' code ' . $e->getCode();
    }
    catch (Exception $e) {
    }

    return $schema_info;
  }


    /**
   * Call the /admin/ping servlet, can be used to quickly tell if a connection to the
   * server is able to be made.
   *
   * @param float $timeout maximum time to wait for ping in seconds, -1 for unlimited (default is 2)
   * @return float Actual time taken to ping the server, FALSE if timeout or HTTP error status occurs
   */
  public function ping($timeout = 2)
  {
    $start = microtime(true);

    $httpTransport = $this->getHttpTransport();

    $httpResponse = $httpTransport->performHeadRequest($this->_pingUrl, $timeout);
    $solrResponse = new Apache_Solr_Response($httpResponse, $this->_createDocuments, $this->_collapseSingleValueArrays);

    if ($solrResponse->getHttpStatus() == 200)
    {
      return microtime(true) - $start;
    }
    else
    {
      return false;
    }
  }

    }
