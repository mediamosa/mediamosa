<?php
/**
 * @file
 * Wrapper class around the AmazonS3 class.
 */

// Make sure class AmazonS3 is loaded.
mediamosa_storage_s3::load_libraries();

class MediaMosaAmazonS3 extends AmazonS3 {

  // ------------------------------------------------------------------ Methods.
	/**
   * Code forked from AmazoneS3.
   *
	 * Creates an Amazon S3 object using the multipart upload APIs. It is
   * analogous to create_object().
	 *
	 * While each individual part of a multipart upload can hold up to 5 GB of
   * data, this method limits the part size to a maximum of 500 MB. The combined
   * size of all parts can not exceed 5 TB of data. When an object is stored in
   * Amazon S3, the data is streamed to multiple storage servers in multiple
   * data centers. This ensures the data remains available in the event of
   * internal network or hardware failure.
	 *
	 * Amazon S3 charges for storage as well as requests to the service. Smaller
   * part sizes (and more requests) allow for faster failures and better upload
   * reliability. Larger part sizes (and fewer requests) costs slightly less but
   * has lower upload reliability.
	 *
	 * In certain cases with large objects, it's possible for this method to
   * attempt to open more file system connections than allowed by the OS. In
   * this case, either increase the number of connections allowed or increase
   * the value of the partSize parameter to use a larger part size.
	 *
	 * @param string $bucket
   *   (Required) The name of the bucket to use.
	 * @param string $filename
   *   (Required) The (destination) file name for the object
   *   (with possible path).
	 * @param array $opt (Optional)
   *   An associative array of parameters that can have the following keys:
	 * 	  - fileUpload string|resource
   *      (Required) The URL/path for the file to upload, or an open resource.
	 * 	  - acl string
   *      (Optional) The ACL settings for the specified object.
   *      Allowed values:
   *        - AmazonS3::ACL_PRIVATE
   *        - AmazonS3::ACL_PUBLIC
   *        - AmazonS3::ACL_OPEN
   *        - AmazonS3::ACL_AUTH_READ
   *        - AmazonS3::ACL_OWNER_READ
   *        - AmazonS3::ACL_OWNER_FULL_CONTROL
   *      The default value is ACL_PRIVATE.
	 * 	  - contentType string
   *      (Optional) The type of content that is being sent in the body. The
   *      default value is application/octet-stream.
	 * 	  - headers array
   *      (Optional) Standard HTTP headers to send along in the request. Accepts
   *      an associative array of key-value pairs.
	 * 	  - length integer
   *      (Optional) The size of the object in bytes. For more information, see
   *      RFC 2616, section 14.13. The value can also be passed to the header
   *      option as Content-Length.
	 * 	  - limit integer
   *      (Optional) The maximum number of concurrent uploads done by cURL. Gets
   *      passed to CFBatchRequest.
	 * 	  - meta array
   *      (Optional) An associative array of key-value pairs. Any header
   *      starting with x-amz-meta-: is considered user metadata. It will be
   *      stored with the object and returned when you retrieve the object. The
   *      total size of the HTTP request, not including the body, must be less
   *      than 4 KB.
	 * 	  - partSize integer
   *      (Optional) The size of an individual part. The size may not be smaller
   *      than 5 MB or larger than 500 MB. The default value is 50 MB.
	 * 	  - seekTo integer
   *      (Optional) The starting position in bytes for the first piece of the
   *      file/stream to upload.
	 * 	  - storage string
   *      (Optional) Whether to use Standard or Reduced Redundancy storage.
   *      Allowed values:
   *        - AmazonS3::STORAGE_STANDARD
   *        - AmazonS3::STORAGE_REDUCED.
   *      The default value is STORAGE_STANDARD.
	 * 	  - uploadId string
   *      (Optional) An upload ID identifying an existing multipart upload to
   *      use. If this option is not set, one will be created automatically.
	 * 	  - curlopts array
   *      (Optional) A set of values to pass  directly into curl_setopt(), where
   *      the key is a pre-defined CURLOPT_* constant.
	 * 	  - returnCurlHandle boolean
   *      (Optional) A private toggle specifying that the cURL handle be
   *      returned rather than actually completing the request. This toggle is
   *      useful for manually managed batch requests.
   *
	 * @return CFResponse
   *   A CFResponse object containing a parsed HTTP response.
   *
	 * @link http://docs.amazonwebservices.com/AmazonS3/latest/dev/RESTAccessPolicy.html REST Access Control Policy
	 */
  public function create_mpu_object($bucket, $filename, $opt = null) {
    if ($this->use_batch_flow) {
      throw new mediamosa_storage_s3_exception(__FUNCTION__ . '() cannot be batch requested');
    }

    if (!$opt) {
      $opt = array();
    }

    // Handle content length. Can also be passed as an HTTP header.
    if (isset($opt['length'])) {
      $opt['headers']['Content-Length'] = $opt['length'];
      unset($opt['length']);
    }

    if (!isset($opt['fileUpload'])) {
      throw new mediamosa_storage_s3_exception('The `fileUpload` option is required in ' . __FUNCTION__ . '().');
    }
    elseif (is_resource($opt['fileUpload'])) {
      $opt['limit'] = 1; // We can only read from this one resource.
      $upload_position = isset($opt['seekTo']) ? (integer) $opt['seekTo'] : ftell($opt['fileUpload']);
      $upload_filesize = isset($opt['headers']['Content-Length']) ? (integer) $opt['headers']['Content-Length'] : null;

      if (!isset($upload_filesize) && $upload_position !== FALSE) {
        $stats = fstat($opt['fileUpload']);

        if ($stats && $stats['size'] >= 0) {
          $upload_filesize = $stats['size'] - $upload_position;
        }
      }
    }
    else {
      $upload_position = isset($opt['seekTo']) ? (integer) $opt['seekTo'] : 0;

      if (isset($opt['headers']['Content-Length']))  {
        $upload_filesize = (integer) $opt['headers']['Content-Length'];
      }
      else {
        $upload_filesize = filesize($opt['fileUpload']);

        if ($upload_filesize !== FALSE) {
           $upload_filesize-= $upload_position;
        }
      }
    }

    if ($upload_position === FALSE || !isset($upload_filesize) || $upload_filesize === FALSE || $upload_filesize < 0) {
      throw new mediamosa_storage_s3_exception('The size of `fileUpload` cannot be determined in ' . __FUNCTION__ . '().');
    }

    // Handle part size.
    if (isset($opt['partSize'])) {
      // If less that 5 MB...
      if ((integer) $opt['partSize'] < 5242880) {
        $opt['partSize'] = 5242880; // 5 MB
      }
      // If more than 500 MB...
      elseif ((integer) $opt['partSize'] > 524288000) {
        $opt['partSize'] = 524288000; // 500 MB
      }
    }
    else {
      $opt['partSize'] = 52428800; // 50 MB
    }

    // If the upload size is smaller than the piece size, failover to
    // create_object().
    if ($upload_filesize < $opt['partSize'] && !isset($opt['uploadId'])) {
      return $this->create_object($bucket, $filename, $opt);
    }

    // Initiate multipart upload.
    if (isset($opt['uploadId'])) {
      $upload_id = $opt['uploadId'];
    }
    else {
      // Compose options for initiate_multipart_upload().
      $_opt = array();
      foreach (array('contentType', 'acl', 'storage', 'headers', 'meta') as $param) {
        if (isset($opt[$param])) {
          $_opt[$param] = $opt[$param];
        }
      }

      $upload = $this->initiate_multipart_upload($bucket, $filename, $_opt);
      if (!$upload->isOK()) {
        return $upload;
      }

      // Fetch the UploadId
      $upload_id = (string) $upload->body->UploadId;
    }

    // Get the list of pieces.
    $pieces = $this->get_multipart_counts($upload_filesize, (integer) $opt['partSize']);

    // Queue batch requests
    $batch = new CFBatchRequest(isset($opt['limit']) ? (integer) $opt['limit'] : null);
    foreach ($pieces as $i => $piece) {
      $this->batch($batch)->upload_part($bucket, $filename, $upload_id, array(
        'expect' => '100-continue',
        'fileUpload' => $opt['fileUpload'],
        'partNumber' => ($i + 1),
        'seekTo' => $upload_position + (integer) $piece['seekTo'],
        'length' => (integer) $piece['length'],
      ));
    }

    // Send batch requests.
    $batch_responses = $this->batch($batch)->send();
    if (!$batch_responses->areOK()) {
      return $batch_responses;
    }

    // Compose completion XML.
    $parts = array();
    foreach ($batch_responses as $i => $response) {
      $parts[] = array('PartNumber' => ($i + 1), 'ETag' => $response->header['etag']);
    }

    return $this->complete_multipart_upload($bucket, $filename, $upload_id, $parts);
  }

  /**
   * Generate a URL for the Amazon S3 object for download.
   *
	 * Gets the web-accessible URL for the Amazon S3 object or generates a
   * time-limited signed request for a private file.
   *
   * Overruled to allow faster lookups.
	 *
	 * @param string $bucket
   *   The name of the bucket to use.
	 * @param string $filename
   *   The file name for the Amazon S3 object.
	 * @param integer|string $preauth
   *   (optional) Specifies that a presigned URL for this request should be
   *   returned. May be passed as a number of seconds since UNIX Epoch, or any
   *   string compatible with strtotime().
	 * @param array $opt
   *   (optional) An associative array;
   *     - https boolean
   *       (optional) Set to TRUE if you would like the URL be in https mode.
   *       Otherwise, the default behavior is always to use http regardless of
   *       your SSL settings.
   *     - method string
   *       (optional) The HTTP method to use for the request. Defaults to a
   *       value of GET.
	 * 	   - response array
   *       (optional) Allows adjustments to specific response headers. Pass an
   *       associative array where each key is one of the following:
   *       cache-control, content-disposition, content-encoding,
   *       content-language, content-type, expires. The expires value should use
   *       gmdate() and be formatted with the DATE_RFC2822 constant.
	 * 	   - torrent boolean
   *       (optional) A value of TRUE will return a URL to a torrent of the
   *       Amazon S3 object. A value of FALSE will return a non-torrent URL.
   *       Defaults to FALSE.
	 * 	   - versionId string
   *       (optional) The version of the object. Version IDs are returned in the
   *       x-amz-version-id header of any previous object-related request.
	 * 	   - returnCurlHandle boolean
   *       (optional) A private toggle specifying that the cURL handle be
   *       returned rather than actually completing the request. This toggle is
   *       useful for manually managed batch requests.
   *
	 * @return string
   *   The file URL, with authentication and/or torrent parameters if requested.
   *
	 * @link http://docs.amazonwebservices.com/AmazonS3/latest/dev/S3_QSAuth.html
   *       Using Query String Authentication.
	 */
//	public function get_object_url($bucket, $filename, $preauth = 0, $opt = NULL) {
//    // Any expire?
//    if ($preauth) {
//      return parent::get_object_url($bucket, $filename, $preauth, $opt);
//    }
//
//    // No expire, create public link.
//    $url = variable_get('mediamosa_storage_s3_object_url', 'http://{bucket}.s3.amazonaws.com/{filename}');
//
//    $replace = array(
//      '{bucket}' => $bucket,
//      '{filename}' => $filename,
//    );
//
//    return strtr($url, $replace);
//  }

  // --------------------------------------------------------- Helper functions.
  /**
   * Check the response.
   *
   * @param CFResponse $response
   *   The response.
   */
  protected function isOKS3Response(CFResponse $response) {
    if ($response === FALSE || !($response instanceof CFResponse)) {
      return FALSE;
    }

    return $response->isOK();
  }

  /**
   * Upload mediafile to S3.
   *
   * @param string $bucket
   *   The bucket name.
   * @param string $source_uri
   *   The uri of the file to upload. URI must be local.
   * @param string $dest_filename
   *   The destination file on S3.
   * @param string $md5
   *   The MD5 of the file. This will be checked after upload against the etag
   *   of the result XML, must match.
   * @param string $mediafile_id
   *   The ID of the mediafile that is uploaded.
   * @param string $content_type
   *   (Optional) Content type of file. When empty, one will be made using the
   *   file extension.
   * @return type
   */
  public function _upload($bucket, $source_uri, $dest_filename, $md5, $mediafile_id, $acl = MediaMosaAmazonS3::ACL_PRIVATE, $content_type = '') {
    // Get the realpath.
    $source_filename = mediamosa_io::realpath($source_uri);

    // Get filename.
    $filename = mediamosa_io::basename($source_filename);

    // Get content type based on extension.
    $content_type = empty($content_type) ? mediamosa_mimetype::extension2mimetype(mediamosa_io::get_ext($filename)) : $content_type;

    $response = $this->create_mpu_object($bucket, $dest_filename, array(
      'fileUpload' => $source_filename,
      'partSize' => 40 * 1024 * 1024,
      'contentType' => $content_type,
      'acl' => $acl,
      'storage' => MediaMosaAmazonS3::STORAGE_STANDARD
    ));

    $md5_upload = trim($response->header['etag'], '"');

    // Check if upload matches our expected md5.
    if ($md5_upload != $md5) {
      // Remove the file, md5 not correct.
      $this->delete_object($bucket, $dest_filename);

      mediamosa_watchdog::log("Upload to S3 failed for mediafile with ID '@id', MD5 checksum did not match.", array('@id' => $mediafile_id));

      // Throw.
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_CHECKSUM_FAILURE, array('@during' => 'upload to Amazon S3', '@id' => $mediafile_id));
    }

    mediamosa_watchdog::log("Upload to S3 successful for mediafile with ID '@id'.", array('@id' => $mediafile_id));

    return $response;
  }

  /**
   * Download a file from S3.
   *
   * @param string $bucket
   *   The bucket name.
   * @param string $source_filename
   *   The complete path of file to download.
   * @param string $dest_filename
   *   The destination path where the file should be stored locally.
   *
   * @return CFResponse
   *   The S3 response object.
   */
  public function _download($bucket, $source_filename, $dest_filename) {
    // Make sure directory exists.
    mediamosa_io::mkdir(mediamosa_io::dirname($dest_filename));

    return $this->get_object($bucket, $source_filename, array(
      'fileDownload' => $dest_filename,
    ));
  }

  /**
   * Get file header info.
   *
   * @param string $bucket
   *   The bucket name.
   * @param string $source_filename
   *   The complete path of file to download.
   *
   * @return CFResponse
   *   The S3 response object.
   */
  public function _headers($bucket, $source_filename) {
    return $this->get_object_headers($bucket, $source_filename);
  }

  /**
   * Get the md5.
   *
   * @param string $bucket
   *   The bucket name.
   * @param string $source_filename
   *   The complete path of file to download.
   *
   * @return string
   *   The MD5 of the file or FALSE.
   */
  public function _getMD5($bucket, $source_filename) {
    // Need MD5.
    $response = $this->_headers($bucket, $source_filename);

    if (!$response->isOK()) {
      return FALSE;
    }

    // Use Etag for md5 contents.
    return trim($response->header['etag'], '"');
  }
}
