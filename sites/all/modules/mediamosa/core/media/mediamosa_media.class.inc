<?php
/**
 * @file
 * Class for /media REST call.
 */

class mediamosa_media {
  // ------------------------------------------------------------------ Consts.
  /**
   * Return uri to the object or file.
   */
  const RESPONSE_URI = 'plain';

  /**
   * Return the uri to the still.
   */
  const RESPONSE_STILL = 'still';

  /**
   * Return, based on container type, the best textual response used for the
   * streaming server.
   */
  const RESPONSE_METAFILE = 'metafile';

  /**
   * Return the object code of the streaming server based on container_type,
   * video_codec and/or mime_type.
   *
   * @see mediamosa_server::get_streaming()
   */
  const RESPONSE_OBJECT = 'object';

  /**
   * Return the downloadable link to the object or file.
   */
  const RESPONSE_DOWNLOAD = 'download';

  // Params in URI replacements.
  const PARAM_URI_TICKET = '{TICKET}';
  const PARAM_URI_SCRIPT = '{SCRIPT}';

  // Default width & height for playproxy flash object HTML.
  const PP_FLASH_OBJECT_HEIGHT = 240;
  const PP_FLASH_OBJECT_WIDTH = 320;

  /**
   * Video codec types.
   */
  const VIDEO_CODEC_H264 = 'h264';

  // ---------------------------------------------------------------- Functions.
  /**
   * Build the filename for source (/data) and symlink (/media).
   *
   * The /data version must not have an extension. The symlink version should
   * have an extension.
   *
   * @param string $basename
   *   The basename of the filename (filename without extension).
   * @param string $extension
   *   (optional) The file extension (without dot).
   * @param string $style_id
   *   (optional) The style_id, used with stills only.
   *
   * @return string
   *   The created mediafile_str.
   */
  public static function filename_build($basename, $extension = '', $style_id = '') {
    return $basename .
      (empty($style_id) ? '' : ',' . $style_id) .
      (empty($extension) ? '' : '.' . $extension);
  }

  /**
   * Split up the filename into it parts.
   *
   * Parts;
   * - basename:
   *   The basename of the filename.
   * - style:
   *   The still style, only when file is still type.
   * - ext
   *   The extension of the filename, without the dot.
   *
   * @param string $filename
   *   The mediafile string; mediafile_id[,style_id][-mediatype][.extension]
   *
   * @return array
   *   An associative array containing;
   *     'basename' (always)
   *     'style_id' (always or 0)
   *     'ext' (no dot).
   */
  public static function filename_parse($filename) {
    $matches = array();
    // Lowercase alphanumeric characters, numbers, underscores (_),
    // and hyphens (-) for style names.
    if (preg_match('@^([a-zA-Z0-9]+),?([a-z_\-0-9]+)?(\.[a-zA-Z0-9]+)?$@', $filename, $matches)) {
      return array(
        $matches[1],
        empty($matches[2]) ? 0 : $matches[2],
        empty($matches[3]) ? '' : mediamosa_unicode::substr($matches[3], 1),
      );
    }

    // If its not in our format, then return it as is.
    return array($filename, 0, '');
  }

  /**
   * Create the symlink.
   *
   * @param string $target
   *   The target of the symlink.
   * @param string $link
   *   The link of the symlink.
   *
   * @throws
   *   mediamosa_exception_error();
   *     ERRORCODE_DIR_NOT_FOUND
   *     ERRORCODE_DIR_NOT_WRITABLE
   *     ERRORCODE_FILE_NOT_FOUND
   *     ERRORCODE_UNABLE_TO_CREATE_SYMLINK
   */
  public static function handle_media_symlink($target, $link) {
    // Create the symlink, if not already. In most cases, its either already
    // been created (mean while during this call) or its the first call.
    if (mediamosa_io::file_exists($link)) {
      return;
    }

    // Get the path.
    $path = mediamosa_io::dirname($link);

    // Create directory.
    mediamosa_io::mkdir($path);

    // Make sure the location is a directory.
    if (!is_dir($path)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_FOUND, array('@location' => $path));
    }

    // Must be able to write there.
    if (!mediamosa_io::is_writable($path)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_WRITABLE, array('@location' => $path));
    }

    // Now create the symlink.
    if (!mediamosa_io::symlink($target, $link)) {
      // Might happen that the symlink fails because it was created during
      // our code between this symlink and !file_exists() above.
      if (!mediamosa_io::file_exists($link)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNABLE_TO_CREATE_SYMLINK);
      }
    }
  }

  /**
   * Serve the file by returning it as file using http.
   *
   * @param string $file
   *   The file location to serve.
   * @param string $mimetype
   *   The mimetype of the file.
   */
  protected static function serve_media($file, $mimetype) {
    // Build up the default headers.
    $headers = mediamosa_io::file_get_content_headers($file, $mimetype);

    // Serve file.
    mediamosa_io::file_transfer($file, $headers);
  }

  /**
   * Process the media request call for /media REST call.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $filename
   *   The filename. May contain style info when file is still.
   */
  public static function handle_media_request_permanent($app_id, $mediafile_id, $filename) {
    // Get the parts.
    list(, $style_id, $extension) = self::filename_parse($filename);

    // Check access. Permanent can not have any ACL protection.
    mediamosa_asset_mediafile::is_mediafile_protected($mediafile_id);

    // Get the mimetype from the technical metadata.
    $mimetype = mediamosa_asset_mediafile::get_mime_type($mediafile_id);

    // Now use the mimetype to match ext.
    $mimetype_extension = mediamosa_mimetype::extension2mimetype($extension);

    // Log it.
    mediamosa_debug::log(
      'Mediafile content-type; @mimetype, matching with extension @extension, @mimetype_extension',
      array(
        '@mimetype' => $mimetype,
        '@extension' => $extension,
        '@mimetype_extension' => $mimetype_extension,
      )
    );

    // The mimetypes must match.
    if ($mimetype_extension != $mimetype) {
      throw new mediamosa_exception_error_mediafile_not_found(array('@mediafile_id' => $mediafile_id));
    }

    // We can trust the extension.
    // At this point the mediafile exists, and we have access.
    //
    // Any style_id?
    if ($style_id) {
      // Handle still style, create it as target for our symlink.
      $target = mediamosa_asset_mediafile_still::image_style_create_derivative($mediafile_id, $style_id);
    }
    else {
      // Get the source file (has no extension) as our symlink target.
      $target = mediamosa_storage::get_uri_mediafile($mediafile_id);
    }

    // The link.
    $link = mediamosa_storage::get_realpath_media_permanent_file($app_id, $mediafile_id, $filename);

    // Create symlink.
    self::handle_media_symlink($target, $link);

    // Serve the file.
    self::serve_media($link, $mimetype);
  }

  /**
   * Proccess the media request using the ticket ID for /media REST call.
   *
   * We only process the ticket when style is provided AND the ticket does
   * exists. The style is only used on stills.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $ticket_id
   *   The ticket ID.
   * @param string $filename
   *   The filename. Still filename may contain style information;
   *   (filename[,style_id][.extension]).
   *
   * @throws mediamosa_exception_error_mediafile_not_found
   * @throws mediamosa_exception_error_invalid_ticket
   */
  public static function handle_media_request_ticket($app_id, $ticket_id, $filename) {
    // Get the parts.
    list($basename, $style_id, $extension) = self::filename_parse($filename);

    // Get the mediafile_id using ticket_id.
    $mediafile_id = mediamosa_media_ticket::get_mediafile_id($ticket_id);

    // No mediafile? Invalid ticket.
    if (empty($mediafile_id)) {
      throw new mediamosa_exception_error_invalid_ticket(array('@ticket_id' => $ticket_id));
    }

    // Get the mimetype from the technical metadata.
    $mimetype = mediamosa_asset_mediafile::get_mime_type($mediafile_id);

    // Now use the mimetype to match ext.
    $mimetype_extension = mediamosa_mimetype::extension2mimetype($extension);

    // The mimetypes must match.
    if ($mimetype_extension != $mimetype) {
      throw new mediamosa_exception_error_invalid_ticket(array('@ticket_id' => $ticket_id));
    }

    // We can trust the extension.
    //
    // Only when the ticket does not exists and a style was defined, then allow
    // to generate the ticket link with style. In any other case, we don't
    // create ticket links.
    //
    // Get the ticket location. It must exist. If it does not, then either the
    // ticket is invalid, timed-out or we don't have access.
    $original_mediaticket_file = mediamosa_storage::get_realpath_media_ticket_file($app_id, $ticket_id, $basename . '/' . $extension);
    if (!mediamosa_io::file_exists($original_mediaticket_file)) {
      throw new mediamosa_exception_error_invalid_ticket(array('@ticket_id' => $ticket_id));
    }

    // Any style?
    if (!$style_id) {
      // If there is no style, then, even with valid ticket, we can not
      // continue. At this point we only allow style versions of files.
      throw new mediamosa_exception_error_mediafile_not_found(array('@mediafile_id' => $mediafile_id));
    }

    // Get (and create) the still derivative using the given style.
    $target = mediamosa_asset_mediafile_still::image_style_create_derivative($mediafile_id, $style_id);

    // Ticket exists and we have a valid style.
    //
    // Now create ticket symlink to the style file. Also the ticket symlink must
    // have the same create date as the original ticket so its cleaned up at the
    // same time as the master ticket.
    //
    // Symlink Link.
    $link = mediamosa_storage::get_realpath_media_ticket_file($app_id, $ticket_id, $filename);

    // Create the symlink.
    self::handle_media_symlink($target, $link);

    // Serve the file.
    self::serve_media($link, $mimetype);
  }

  /**
   * Return the filename to use for permanent and ticket links.
   *
   * @param array $mediafile
   *   The mediafile.
   *
   * @return string
   *   The filename.
   */
  public static function get_media_filename(array $mediafile) {
    // Get filename.
    $filename = mediamosa_asset_mediafile::get_filename($mediafile);

    // Get the file without ext.
    $filename = mediamosa_io::get_base_filename($filename);

    // Now get the ext. based on mimetype/container type.
    $ext = mediamosa_asset_mediafile::get_file_extension($mediafile);

    // Attach extension to filename.
    return $filename . (!empty($ext) ? '.' . $ext : '');
  }

  /**
   * Process ticket for media view usage. Will setup the symlink to the file.
   *
   * @param integer $app_id
   *   Application ID.
   * @param string $user_id
   *   The owner for possible ticket.
   * @param array $mediafile
   *   The mediafile to process.
   * @param string $response_type
   *   See mediamosa_media::RESPONSE_*
   * @param boolean $is_app_admin
   *   Is app admin.
   *
   * @return array
   *   An associative array; (see mediamosa_io_streamwrapper::MEDIA_VIEW_*)
   *   - 'ticket_id'
   *     The ticket ID used.
   *   - 'link'
   *     The symbolic link complete path.
   *   - 'filename'
   *     The filename of the media.
   *   - 'server_uri_build'
   *     The finished server uri.
   */
  public static function process_media_view($app_id, $user_id, array $mediafile, $response_type, $is_app_admin) {
    // Get normal uri to mediafile.
    $mediafile_uri = mediamosa_storage::get_uri_mediafile($mediafile);

    // Need streamwrapper.
    $wrapper = mediamosa_io::require_stream_wrapper_instance_by_uri($mediafile_uri);

    // Create the media view.
    return $wrapper->media_view($app_id, $user_id, $mediafile, $response_type, $is_app_admin);
  }

  /**
   * Return the server URI of the mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile to get server uri for.
   * @param string $uri
   *   The mediafile URI.
   * @param string $response_type
   *   The response type in case mediafile URI is not set.
   *
   * @return string
   *   The (streaming) server URI.
   */
  public static function get_server_uri($mediafile_id, $uri, $response_type) {
    if (trim($uri) != '') {
      return trim($uri);
    }

    return mediamosa_server::get_server_by_responsetype($response_type, $mediafile_id);
  }

  /**
   * Convert response type into ticket type.
   *
   * @param string $response_type
   *   The response type to convert.
   *
   * @return string
   *   The ticket type.
   */
  public static function responsetype2tickettype($response_type) {
    $responsetype2tickettype = array(
      mediamosa_media::RESPONSE_STILL => mediamosa_media_ticket::TICKET_TYPE_STILL,
      mediamosa_media::RESPONSE_DOWNLOAD => mediamosa_media_ticket::TICKET_TYPE_DOWNLOAD,
      mediamosa_media::RESPONSE_METAFILE => mediamosa_media_ticket::TICKET_TYPE_VIEW,
      mediamosa_media::RESPONSE_OBJECT => mediamosa_media_ticket::TICKET_TYPE_VIEW,
      mediamosa_media::RESPONSE_URI => mediamosa_media_ticket::TICKET_TYPE_VIEW,
    );

    assert(isset($responsetype2tickettype[$response_type]));

    // Convert to ticket type.
    return $responsetype2tickettype[$response_type];
  }

  /**
   * Process response for download media view call.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The owner of the ticket.
   * @param array $mediafile
   *   The mediafile to download.
   * @param boolean $is_app_admin
   *   Is user application admin.
   *
   * @return array
   *   An associative array;
   *   - 'output'
   *     The output value for the response.
   *   - 'content_type'
   *     The mime-type of the target of the output.
   *   - 'ticket_id'
   *     The ticket ID created.
   *
   * @throws mediamosa_exception_error
   */
  public static function do_response_download($app_id, $user_id, $is_app_admin, $mediafile) {
    if (!$is_app_admin) {
      if (!mediamosa_lib::boolstr2bool($mediafile[mediamosa_asset_mediafile_db::IS_DOWNLOADABLE])) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_DOWNLOAD_DISABLED);
      }
    }

    // Build the mediaview.
    $media_view = mediamosa_media::process_media_view($app_id, $user_id, $mediafile, self::RESPONSE_DOWNLOAD, $is_app_admin);

    return array(
      'output' => $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_SERVER_URI_BUILD],
      'content_type' => mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile[mediamosa_asset_mediafile_db::ID], 'mime_type'),
      'ticket_id' => $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID],
    );
  }

  /**
   * Process response for metafile media view call.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The owner of the ticket.
   * @param array $mediafile
   *   The mediafile to use.
   * @param boolean $is_app_admin
   *   Is user application admin.
   * @param boolean $autostart
   *   The autostart parameter.
   * @param integer $start
   *   The start parameter.
   * @param integer $duration
   *   The duration parameter.
   *
   * @return array
   *   An associative array;
   *   - 'output'
   *     The output value for the response.
   *   - 'content_type'
   *     The mime-type of the target of the output.
   *   - 'ticket_id'
   *     The ticket ID created.
   *
   * @throws mediamosa_exception_error
   */
  public static function do_response_metafile($app_id, $user_id, $is_app_admin, $mediafile, $autostart, $start, $duration) {
    // Get the asset_id.
    $asset_id = $mediafile[mediamosa_asset_mediafile_db::ASSET_ID];

    // Build the mediaview.
    $media_view = mediamosa_media::process_media_view($app_id, $user_id, $mediafile, self::RESPONSE_METAFILE, $is_app_admin);

    // Get server uri build.
    $server_uri_build = $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_SERVER_URI_BUILD];

    // Get the container type.
    $container_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile[mediamosa_asset_mediafile_db::ID], 'container_type');

    switch ($container_type) {
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_MP4:
        // Get video codec.
        $video_codec = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile[mediamosa_asset_mediafile_db::ID], 'video_codec');

        if ($video_codec == self::VIDEO_CODEC_H264) {
          $content_type = 'application/smil';

          // Get the rtmp (wowza) server URI.
          // URI should looks like:
          // rtmp://wowstreaming.test.snkn.nl/vod&file=vpx-acc/{TICKET}
          // rtmp://wowstreaming.snkn.nl/simplevideostreaming&file=vpx/{TICKET}
          // Separate the URL into two parts:
          // "rtmp://wowstreaming.test.snkn.nl/vod" and "vpx-acc/{TICKET}"
          $exploded_uri = explode('&file=', $server_uri_build);
          if (isset($exploded_uri[1])) {
            $meta_base = $exploded_uri[0];
            $video_src = $exploded_uri[1];
          }
          else {
            // Apache server.
            // http://mediamosa2/ticket/LRQKJp4RRHdiOQjZJAiV2rOP.flv
            // Separation by the last '/':
            // "http://mediamosa2/ticket/" and "LRQKJp4RRHdiOQjZJAiV2rOP.flv"
            $meta_base = mediamosa_io::dirname($server_uri_build);
            $video_src = mediamosa_io::basename($server_uri_build);
          }
          // Create the output.
          $output = '<smil>' . "\n";
          $output .= '  <head>' . "\n";
          $output .= '    <meta base="' . $meta_base . '" />' . "\n";
          $output .= '  </head>' . "\n";
          $output .= '  <body>' . "\n";
          $output .= '    <video src="' . $video_src . '" />' . "\n";
          $output .= '  </body>' . "\n";
          $output .= '</smil>';
        }
        else {
          $content_type = 'application/x-quicktimeplayer';
          $output = '<?xml version="1.0"?>' . "\n";
          $output .= '<?quicktime type="application/x-quicktime-media-link"?>' . "\n";
          $output .= sprintf("<embed autoplay=\"%s\" src=\"%s\" />\n", $autostart ? 'true' : 'false', $server_uri_build);
        }
        break;

      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_MP3:
        // Get asset metadata.
        $asset_metadata = mediamosa_asset_metadata::metadata_get($asset_id);

        $content_type = 'audio/mpeg';
        $output = "<asx version=\"3.0\">\n\t<entry>\n";

        if (!empty($asset_metadata['dublin_core'])) {
          foreach (array(
            'TITLE' => 'title',
            'COPYRIGHT' => 'rights',
            'ABSTRACT' => 'description',
            'AUTHOR' => 'creator') as $tag => $subject) {
            if (!empty($asset_metadata['dublin_core'][$subject])) {
              $output .= "\t\t<" . $tag . '>' . implode(' ', $asset_metadata['dublin_core'][$subject]['values']) . '</' . $tag . ">\n";
            }
          }
        }

        $output .= sprintf("\t\t<ref href=\"%s\" />\n", $server_uri_build);
        $output .= "\t</entry>\n</asx>\n";
        break;

      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV:
        // Get asset metadata.
        $asset_metadata = mediamosa_asset_metadata::metadata_get($asset_id);

        $content_type = 'video/x-ms-asf';
        $output = "<asx version=\"3.0\">\n\t<entry>\n";

        if (!empty($asset_metadata['dublin_core'])) {
          foreach (array(
            'TITLE' => 'title',
            'COPYRIGHT' => 'rights',
            'ABSTRACT' => 'description',
            'AUTHOR' => 'creator') as $tag => $subject) {
            if (!empty($asset_metadata['dublin_core'][$subject])) {
              $output .= "\t\t<" . $tag . '>' . implode(' ', $asset_metadata['dublin_core'][$subject]['values']) . '</' . $tag . ">\n";
            }
          }
        }

        // Add start and duration to asx.
        if (isset($start)) {
          $output .= sprintf("\t\t<starttime value=\"%s\" />\n", self::convert_msec($start, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV));
        }

        if (isset($duration)) {
          $output .= sprintf("\t\t<duration value=\"%s\" />\n", self::convert_msec($duration, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV));
        }

        $output .= sprintf("\t\t<ref href=\"%s\" />\n", $server_uri_build);
        $output .= "\t</entry>\n</asx>\n";
        break;

      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_FLV:
        $content_type = 'application/smil';

        // Get the rtmp (wowza) server URI.
        // URI should looks like:
        // rtmp://wowstreaming.test.snkn.nl/vod&file=vpx-acc/{TICKET}
        // rtmp://wowstreaming.snkn.nl/simplevideostreaming&file=vpx/{TICKET}
        // Separate the URL into two parts:
        // "rtmp://wowstreaming.test.snkn.nl/vod" and "vpx-acc/{TICKET}"
        $exploded_uri = explode('&file=', $server_uri_build);
        if (isset($exploded_uri[1])) {
          $meta_base = $exploded_uri[0];
          $video_src = $exploded_uri[1];
        }
        else {
          // Apache server.
          // http://mediamosa2/ticket/LRQKJp4RRHdiOQjZJAiV2rOP.flv
          // Separation by the last '/':
          // "http://mediamosa2/ticket/" and "LRQKJp4RRHdiOQjZJAiV2rOP.flv"
          $meta_base = mediamosa_io::dirname($server_uri_build);
          $video_src = mediamosa_io::basename($server_uri_build);
        }
        // Create the output.
        $output = '<smil>' . "\n";
        $output .= '  <head>' . "\n";
        $output .= '    <meta base="' . $meta_base . '" />' . "\n";
        $output .= '  </head>' . "\n";
        $output .= '  <body>' . "\n";
        $output .= '    <video src="' . $video_src . '" />' . "\n";
        $output .= '  </body>' . "\n";
        $output .= '</smil>';
        break;

      default:
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_METAFILE_AVAILABLE, array('@container_type' => $container_type));
    }

    return array(
      'output' => $output,
      'content_type' => $container_type,
      'ticket_id' => $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID],
    );
  }

  /**
   * Process response for object media view call.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The owner of the ticket.
   * @param array $mediafile
   *   The mediafile to use.
   * @param boolean $is_app_admin
   *   Is app admin.
   * @param integer $width
   *   The width of the video.
   * @param integer $height
   *   The height of the video.
   * @param boolean $autostart
   *   The autostart parameter.
   * @param integer $start
   *   The start parameter.
   * @param integer $duration
   *   The duration parameter.
   *
   * @return array
   *   An associative array;
   *   - 'output'
   *     The output value for the response.
   *   - 'content_type'
   *     The mime-type of the target of the output.
   *   - 'ticket_id'
   *     The ticket ID created.
   *
   * @throws mediamosa_exception_error
   */
  public static function do_response_object($app_id, $user_id, $is_app_admin, $mediafile, $width, $height, $autostart, $start, $duration) {
    // Build the mediaview.
    $media_view = mediamosa_media::process_media_view($app_id, $user_id, $mediafile, self::RESPONSE_OBJECT, $is_app_admin);

    // Get server uri build.
    $server_uri_build = $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_SERVER_URI_BUILD];

    // Default type for objects.
    $content_type = 'text/html';

    // Get the container type.
    $container_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile[mediamosa_asset_mediafile_db::ID], 'container_type');

    // Get mediafile width and height from technical metadata.
    $mediafile_width = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($mediafile[mediamosa_asset_mediafile_db::ID], 'width');
    $mediafile_height = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($mediafile[mediamosa_asset_mediafile_db::ID], 'height');

    // Calculate the video size.
    $video_size = self::calc_video_size(
      empty($mediafile_width) ? 0 : $mediafile_width,
      empty($mediafile_height) ? 0 : $mediafile_height,
      $width,
      $height,
      isset($container_type) ? $container_type : mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV
    );

    // Get mediafile metadata.
    $video_codec = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile[mediamosa_asset_mediafile_db::ID], 'video_codec');
    $mime_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile[mediamosa_asset_mediafile_db::ID], 'mime_type');

    // Get the Object code.
    $output = mediamosa_server::get_objectcode_streaming($container_type, $video_codec, $mime_type);

    $variables = array(
      '{WIDTH}' => !$video_size['width'] ? '' : $video_size['width'],
      '{HEIGHT}' => !$video_size['height'] ? '' : $video_size['height'],
      '{HEIGHT_PLUS_20}' => $video_size['height'] > 0 ? $video_size['height'] + 20 : '',
      '{MEDIAFILE_ID}' => $mediafile[mediamosa_asset_mediafile_db::ID],
      '{TICKET_URI}' => $server_uri_build,
      '{STILL_URI}' => self::do_response_still_default($app_id, $mediafile[mediamosa_asset_mediafile_db::ASSET_ID]),
      '{AUTOPLAY}' => $autostart ? 'true' : 'false',
      '{AUTOPLAY_NUM}' => $autostart ? '1' : '0',
      '{AUTOPLAY_TEXT}' => $autostart ? 'autoplay' : '',
      '{IF_START}' => '',
      '{/IF_START}' => '',
      '{IF_DURATION}' => '',
      '{/IF_DURATION}' => '',
      '{IF_EXTERNAL}' => '',
      '{/IF_EXTERNAL}' => '',
    );

    // HEIGHT_PLUS_num.
    $pattern = '/{HEIGHT_PLUS_(\d+)}/';
    $output = preg_replace_callback(
      $pattern,
      create_function(
        '$matches',
        'return ' . ($video_size['height'] > 0 ? $video_size['height'] . ' + $matches[1]' : '""') . ';'
      ),
      $output
    );

    // START, END DURATION TIMES variables.
    if (isset($start)) {
      $variables['{START_TIME}'] = self::convert_msec($start, $container_type);
      $variables['{START_TIME_SECONDS}'] = (int) ($start / 1000);
    }
    else {
      // Remove code {IF_START} (Exclude from output)
      // \{IF_START\}(.*?)\{/IF_START\}/is.
      $output = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $output);
    }

    if (isset($duration)) {
      $endtime = $start + $duration;
      $variables['{DURATION_TIME}'] = self::convert_msec($duration, $container_type);
      $variables['{DURATION_TIME_SECONDS}'] = (int) ($duration / 1000);
      $variables['{START_PLUS_DURATION_TIME_SECONDS}'] = (int) ($start + $duration) / 1000;
      $variables['{END_TIME}'] = self::convert_msec($endtime, $container_type);
    }
    else {
      $output = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $output);
    }

    switch ($container_type) {
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_FLV:
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_MP4:
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_MP3:
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_OGG:
        break;

      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV:
      default:
        // Either set, then we must use an external asx file (asx requires this
        // when start or duration is used.
        if (isset($start) || isset($duration)) {
          $input = array();
          $output_asx = '';
          preg_match('@\{IF_EXTERNAL\}(.*?)\{/IF_EXTERNAL\}@is', $output, $input);
          if (isset($input[0])) {
            $output_asx = $input[0];
          }
          else {
            mediamosa_watchdog::log('Object code for ASX is missing the {IF_EXTERNAL} ... {/IF_EXTERNAL} tags, unable to generate .asx file for external usage.', array(), WATCHDOG_CRITICAL, 'playproxy');
          }
        }

        // Got output template for asx?
        if (!empty($output_asx)) {
          if (isset($start)) {
            // Remove code {IF_START} (Exclude from output).
            $variables['{START_TIME}'] = self::convert_msec($start, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV);
          }
          else {
            $output_asx = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $output_asx);
          }

          if (isset($duration)) {
            $variables['{DURATION_TIME}'] = self::convert_msec($duration, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV);
          }
          else {
            $output_asx = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $output_asx);
          }

          // Process all the variables and make replacement in the objectcode.
          $output_asx = strtr($output_asx, $variables);

          // Same filename, extension .asx.
          $filename_asx = mediamosa_io::get_base_filename(mediamosa_io::basename($media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_FILENAME])) . '.asx';

          // Get the filename for the asx file.
          $file_asx = mediamosa_storage::get_realpath_object_file($app_id, $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID], $filename_asx);

          // Create a possible app_id directory.
          mediamosa_io::mkdir(mediamosa_io::dirname($file_asx));

          // Open file for writing.
          $handle_file_asx = @fopen($file_asx, 'w+');

          // Check if file handle is open, else log it.
          if ($handle_file_asx) {
            // Write to file.
            $bytes_written = @fwrite($handle_file_asx, $output_asx);

            // Close file handle.
            fclose($handle_file_asx);

            // Any bytes written.
            if ($bytes_written) {
              // Get the streaming server URI.
              $server_uri = self::get_server_uri($mediafile[mediamosa_asset_mediafile_db::ID], '', self::RESPONSE_STILL);

              // Path to object file.
              $path = mediamosa_storage::get_path_media_object_file($app_id, $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID], $filename_asx);

              // Rebuild uri with path.
              $server_uri_build = strtr($server_uri, array(self::PARAM_URI_TICKET => $path));

              // Is a public link?
              if ($media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_IS_PUBLIC]) {
                $ticket_id = $mediafile[mediamosa_asset_mediafile_db::ID];

                // Get the paths needed for the symlink.
                $link = mediamosa_storage::get_realpath_media_permanent_file($app_id, $ticket_id, $filename_asx);
              }
              else {
                // Convert to ticket type.
                $ticket_type = mediamosa_media::responsetype2tickettype(self::RESPONSE_OBJECT);

                // Get the paths needed for the symlink.
                $link = mediamosa_storage::get_realpath_media_ticket_file($app_id, $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID], $filename_asx, $ticket_type);
              }

              // The current symlink now points to the mediafile, must change it
              // to the object file.
              if (!empty($media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_LINK])) {
                mediamosa_io::unlink($media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_LINK]);
              }

              // Add symlink, if not already (= perm).
              if (!mediamosa_io::file_exists($link)) {
                // Create new symlink.
                if (!mediamosa_io::symlink($file_asx, $link)) {
                  throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNABLE_TO_CREATE_SYMLINK, array('@location' => $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_LINK]));
                }
              }
            }
            else {
              mediamosa_watchdog::log('Unable to write to .asx file (@filename), falling back to normal object code (1)', array('@filename' => $file_asx), WATCHDOG_CRITICAL, 'playproxy');
            }
          }
          else {
            mediamosa_watchdog::log('Unable to create .asx file (@filename), falling back to normal object code (2)', array('@filename' => $file_asx), WATCHDOG_CRITICAL, 'playproxy');
          }
        }

        // Remove code {IF_EXTERNAL} (Exclude from output).
        $output = preg_replace('@\{IF_EXTERNAL\}.*?\{/IF_EXTERNAL\}@si', '', $output);

        // Add WMA ticket uri for asx.
        $variables['{WMA_TICKET_URI}'] = $server_uri_build;
        break;
    }

    // Process all the variables and make replacement in the objectcode.
    $output = strtr($output, $variables);

    return array(
      'output' => $output,
      'content_type' => $content_type,
      'ticket_id' => $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID],
    );
  }

  /**
   * Process response for still media view call.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The user ID for ticket owner.
   * @param array $still
   *   The still mediafile.
   * @param boolean $is_app_admin
   *   Is app admin.
   * @param array $filtered_stills
   *   The filtered stills result or empty.
   *
   * @return array
   *   An associative array;
   *   - 'output'
   *     The output value for the response.
   *   - 'content_type'
   *     The mime-type of the target of the output.
   *   - 'ticket_id'
   *     The ticket ID created.
   *
   * @throws mediamosa_exception_error
   */
  public static function do_response_still($app_id, $user_id, $is_app_admin, $still, array $filtered_stills = array()) {

    // Build the mediaview.
    $media_view = mediamosa_media::process_media_view($app_id, $user_id, $still, mediamosa_media::RESPONSE_STILL, $is_app_admin);

    // Go through the filtered stills and copy main still and replace the
    // version in filtered_stills.
    foreach ($filtered_stills as $key => $filtered_still) {
      if ($still[mediamosa_asset_mediafile_db::ID] == $filtered_still['still_id']) {
        $filtered_media_view = $media_view;
      }
      else {
        $filtered_still[mediamosa_asset_mediafile_db::ID] = $filtered_still['still_id'];
        unset($filtered_still['still_id']);
        $filtered_media_view = mediamosa_media::process_media_view($app_id, $user_id, $filtered_still, mediamosa_media::RESPONSE_STILL, $is_app_admin);
      }

      // Add still ticket.
      $filtered_stills[$key]['still_ticket'] = $filtered_media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_SERVER_URI_BUILD];
      $filtered_stills[$key]['ticket'] = $filtered_media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID];
    }

    $response = array(
      'output' => $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_SERVER_URI_BUILD],
      'content_type' => mediamosa_asset_mediafile::get_mime_type($still[mediamosa_asset_mediafile_db::ID]),
      'ticket_id' => $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID],
    );

    // Add possible stills to response.
    $i = 1;
    foreach ($filtered_stills as $key => $filtered_still) {
      $response['stills']['#' . serialize(array('id' => $i++))] = $filtered_still;
    }

    return $response;
  }

  /**
   * Generate default still link for asset.
   *
   * @param integer $app_id
   *   The client application ID.
   * @param string $asset_id
   *   The asset ID to use default still.
   * @param boolean $is_app_admin
   *   Is app admin.
   *
   * @return string
   *   The still response url.
   */
  public static function do_response_still_default($app_id, $asset_id, $is_app_admin = FALSE) {
    // Get default still.
    $still = mediamosa_asset_mediafile_still::find_default($asset_id);

    // Any still?
    if (!empty($still)) {
      $response = self::do_response_still($app_id, $still[mediamosa_asset_mediafile_db::OWNER_ID], $is_app_admin, $still);
      return $response['output'];
    }

    return '';
  }

  /**
   * Process response for still uri (plain) view call.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The user ID for ticket owner.
   * @param array $mediafile
   *   The mediafile.
   * @param boolean $is_app_admin
   *   Is app admin.
   *
   * @return array
   *   An associative array;
   *   - 'output'
   *     The output value for the response.
   *   - 'content_type'
   *     The mime-type of the target of the output.
   *   - 'ticket_id'
   *     The ticket ID created.
   */
  public static function do_response_uri($app_id, $user_id, $is_app_admin, $mediafile) {

    // Build the mediaview.
    $media_view = mediamosa_media::process_media_view($app_id, $user_id, $mediafile, mediamosa_media::RESPONSE_URI, $is_app_admin);

    // Get the container type.
    $container_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile[mediamosa_asset_mediafile_db::ID], 'container_type');

    // Get the uri.
    $output = $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_SERVER_URI_BUILD];
    if ($container_type == mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_FLV) {
      $output = strtr($output, array(self::PARAM_URI_SCRIPT => 'StroboScope.php?file='));
    }

    return array(
      'output' => $output,
      'content_type' => mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile[mediamosa_asset_mediafile_db::ID], 'mime_type'),
      'ticket_id' => $media_view[mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID],
    );
  }

  /**
   * Test if a perminant mediafile link can be made, is only available for
   * public mediafiles.
   *
   * @param array $mediafile
   *   The mediafile to test.
   * @param boolean $is_app_admin
   *   Is app admin.
   *
   * @throws mediamosa_exception_error_is_inappropriate
   */
  public static function is_public(array $mediafile, $is_app_admin) {
    // Protected mediafiles can not be permanent.
    try {
      return !mediamosa_asset_mediafile::is_mediafile_protected_ext($mediafile);
    }
    catch (mediamosa_exception_error_is_inappropriate $e) {
      if (!$is_app_admin) {
        throw $e;
      }
      return FALSE;
    }
    catch (mediamosa_exception $e) {
      return FALSE;
    }
  }

  /**
   * Remove the permanent symlink to a mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile to remove link from.
   */
  public static function remove_public_link($app_id, $mediafile_id) {
    $media_location = mediamosa_storage::get_realpath_media_permanent_file($app_id);
    if (mediamosa_io::file_exists($media_location)) {
      // Remove permanent links of mediafile.
      mediamosa_io::exec(
        strtr(
          'find @media_location -maxdepth 3 -mindepth 3 -name "@wildmatch*" -type l -delete',
          array(
            '@media_location' => $media_location,
            '@wildmatch' => $mediafile_id,
          )
        )
      );
    }
  }

  /**
   * Remove the permanent symlink to all mediafiles of provided asset.
   *
   * @param string $asset_id
   *   The asset ID.
   */
  public static function remove_public_link_asset($asset_id) {

    // Get the mediafiles.
    $mediafiles = mediamosa_asset_mediafile::get_by_asset_id(
      $asset_id,
      array(
        mediamosa_asset_mediafile_db::APP_ID,
        mediamosa_asset_mediafile_db::ID,
      )
    );

    // Remove all from asset.
    foreach ($mediafiles as $mediafile_id => $mediafile) {
      self::remove_public_link($mediafile[mediamosa_asset_mediafile_db::APP_ID], $mediafile_id);
    }
  }

  /**
   * This function converts miliseconds to the format specified.
   *
   * @param integer $input
   *   The number of seconds.
   * @param string $container_type
   *   See CONTAINER_TYPE_*.
   *
   * @return string
   *   Converted time in msec format.
   */
  public static function convert_msec($input, $container_type) {
    $output = FALSE;

    // Calculate hour, minute and seconds.
    $second = 1000;
    $minute = $second * 60;
    $hour = $minute * 60;
    $output = array();
    $output['hours'] = floor($input / $hour);
    $input -= $output['hours'] * $hour;
    $output['minutes'] = floor($input / $minute);
    $input -= $output['minutes'] * $minute;
    $output['seconds'] = floor($input / $second);
    $input -= $output['seconds'] * $second;
    $msec = $input;

    switch ($container_type) {
      // hh:mm:ss.fract(=sec/100).
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV:
        $fraction = mediamosa_unicode::substr($msec, 0, 2);
        foreach ($output as $subject => $value) {
          if ($output[$subject] < 10) {
            $output[$subject] = '0' . $output[$subject];
          }
        }
        $output = implode(':', $output) . '.' . $fraction;
        break;

      // hh:mm:ss:frames(=sec/30).
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_MP4:
      default:
        $frame = 1 / 30;
        $fraction = round($msec / $frame / 1000, 1);
        if ($fraction == 30) {
          $fraction = 0;
          $output['seconds']++;
        }

        foreach ($output as $subject => $value) {
          if ($output[$subject] < 10) {
            $output[$subject] = '0' . $output[$subject];
          }
        }
        $output = implode(':', $output) . ':' . $fraction;
        break;
    }

    return $output;
  }

  /**
   * Calculate the size of the video based on given size and type.
   *
   * @param integer $video_width
   *   The mediafile width.
   * @param integer $video_height
   *   The mediafile height.
   * @param integer $width
   *   The media play call parameter width.
   * @param integer $height
   *   The media play call parameter height.
   * @param string $container_type
   *   See mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_*.
   *
   * @return array
   *   The new width en height.
   */
  public static function calc_video_size($video_width, $video_height, $width, $height, $container_type) {

    // If video size is known, we can do some calculations on height/weight.
    if ($video_width > 0 && $video_height > 0) {
      if ($width > 0 && ($height == 0 || !isset($height))) {
        $height = (int) round($width * ($video_height / $video_width));
      }
      elseif (($width == 0 || !isset($width)) && $height > 0) {
        $width = (int) round($height * ($video_width / $video_height));
      }
      elseif (($width == 0 || !isset($width)) && ($height == 0 || !isset($height))) {
        // If none is given we use the default size.
        $width = $video_width;
        $height = $video_height;
      }
    }

    if ($width <= 0 && $height <= 0) {
      // No luck, use default values.
      $width = self::PP_FLASH_OBJECT_WIDTH;
      $height = self::PP_FLASH_OBJECT_HEIGHT;
    }

    switch ($container_type) {
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_FLV:
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_MP4:
      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_OGG:
      default:
        break;

      case mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV:
        // Add an extra 64 pixels for windows media buttons.
        if ($height) {
          $height += 64;
        }
        break;
    }

    return array('width' => $width, 'height' => $height);
  }

  /**
   * Check if the time has been restricted.
   *
   * @param integer $time_restriction_start
   *   Start time in UTC of the time restriction period in datetime format
   *   (optional).
   * @param integer $time_restriction_end
   *   End time in UTC of the time restriction period date.
   *
   * @throws mediamosa_exception_error()
   */
  public static function check_time_restrictions($time_restriction_start, $time_restriction_end) {
    if (time() < $time_restriction_start) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_TIME_RESTRICTION_START, array(
        '@date' => date('Y-m-d H:i:s', $time_restriction_start),
        '@timestamp' => $time_restriction_start,
      ));
    }
    if (time() > $time_restriction_end) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_TIME_RESTRICTION_END, array(
        '@date' => date('Y-m-d H:i:s', $time_restriction_end),
        '@timestamp' => $time_restriction_end,
      ));
    }
  }

  /**
   * Return the stills based on filter properties.
   *
   * If filter properties are empty then all stills of this asset are returned.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $asset_id
   *   The asset ID.
   * @param string $mediafile_id
   *   (optional) The mediafile ID.
   * @param string $still_id
   *   (optional) The still ID to check against filter parameters.
   * @param string $size
   *   (optional) In WxH format.
   * @param integer $width
   *   (optional) The width.
   * @param integer $height
   *   (optional) The height.
   * @param string $range
   *   (optional) The range of stills ('1' or '1,3-5').
   * @param string $format
   *   (optional) The still format.
   * @param string $tag
   *   (optional) The tag on still(s).
   *
   * @return array
   *   The found stills.
   *
   * @throws mediamosa_exception_error
   */
  public static function collect_stills($app_id, $asset_id, $mediafile_id, $still_id, $size, $width, $height, $range, $format, $tag) {
    // Stills to create.
    $stills = array();

    // Get width and height.
    list($width, $height) = mediamosa_asset_mediafile_still::process_image_size($size, $width, $height, mediamosa_asset_mediafile_still::get_default_image_size($app_id));

    // Get the order, if any.
    $order = mediamosa_asset_mediafile_still::process_still_range($range);

    // Now query mediafiles and collect the stills.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'still');
    $query->join(
      mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
      'mfmm',
      'mfmm.mediafile_id = still.mediafile_id'
    );
    $query->join(
      mediamosa_asset_mediafile_db::TABLE_NAME,
      'mf',
      'mf.mediafile_id = still.mediafile_id_source AND mf.is_still = :is_still',
      array('is_still' => mediamosa_asset_mediafile_db::IS_STILL_FALSE)
    );
    $query->addField('still', mediamosa_asset_mediafile_db::ID, 'still_id');
    $query->fields('still',
      array(
        mediamosa_asset_mediafile_db::ASSET_ID,
        mediamosa_asset_mediafile_db::APP_ID,
        mediamosa_asset_mediafile_db::OWNER_ID,
        mediamosa_asset_mediafile_db::FILENAME,
        mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
        mediamosa_asset_mediafile_db::TAG,
        mediamosa_asset_mediafile_db::URI,
        mediamosa_asset_mediafile_db::IS_PROTECTED,
      )
    );
    $query->addField('mf', mediamosa_asset_mediafile_db::ID, 'mediafile_id');

    $query->condition('still.' . mediamosa_asset_mediafile_db::ASSET_ID, $asset_id);
    $query->condition('still.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE);
    $query->condition('still.' . mediamosa_asset_mediafile_db::APP_ID, $app_id);

    if (!empty($still_id)) {
      $query->condition('still.' . mediamosa_asset_mediafile_db::ID, $still_id);
    }
    if (!empty($mediafile_id)) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
    }
    if (isset($tag)) {
      $query->condition('still.' . mediamosa_asset_mediafile_db::TAG, $tag);
    }

    // Order by still order.
    $query->condition('mfmm.' . mediamosa_asset_mediafile_metadata_db::PROP_ID, mediamosa_asset_mediafile_metadata_property::get_property_id_int(mediamosa_asset_mediafile_metadata::STILL_ORDER));
    $query->orderBy('mfmm.' . mediamosa_asset_mediafile_metadata_db::VAL_INT, 'ASC');

    foreach ($query->execute() as $still) {
      // Enrich the t_stills with the still parameters.
      if (self::collect_stills_helper($still, $width, $height, $format, $order)) {
        $stills[] = $still;
      }
    }

    return $stills;
  }

  /**
   * Select still based on given parameters.
   *
   * Match provided width, height, format and order to select the correct
   * stills.
   *
   * @param array &$still
   *   The mediafile still array where its metadata will be added.
   * @param integer $width
   *   (optional) The width.
   * @param integer $height
   *   (optional) The height.
   * @param string $format
   *   (optional) The format.
   * @param integer $order
   *   (optional) The order in stills.
   *
   * @return boolean
   *   Returns TRUE when still matches the provided values (width, height,
   *   format, order).
   */
  static protected function collect_stills_helper(array &$still, $width = NULL, $height = NULL, $format = NULL, $order = NULL) {

    // Need this to compare.
    if (!isset($width) && !isset($height) && !isset($format) && !isset($order)) {
      return TRUE;
    }

    // Get the technical metadata of the still.
    $values = mediamosa_asset_mediafile_metadata::get_all_mediafile_metadata($still['still_id']);

    $fields = array(
      mediamosa_asset_mediafile_metadata::WIDTH,
      mediamosa_asset_mediafile_metadata::HEIGHT,
      mediamosa_asset_mediafile_metadata::FILESIZE,
      mediamosa_asset_mediafile_metadata::MIME_TYPE,
      mediamosa_asset_mediafile_metadata::STILL_TIME_CODE,
      mediamosa_asset_mediafile_metadata::STILL_ORDER,
      mediamosa_asset_mediafile_metadata::STILL_FORMAT,
      mediamosa_asset_mediafile_metadata::STILL_TYPE,
      mediamosa_asset_mediafile_metadata::STILL_DEFAULT,
    );

    // Collect the values we need.
    foreach ($fields as $field) {
      $still[$field] = (empty($values[$field]) ? NULL : $values[$field]);
    }

    if (isset($width) && $width != $still[mediamosa_asset_mediafile_metadata::WIDTH]) {
      return FALSE;
    }
    if (isset($height) && $height != $still[mediamosa_asset_mediafile_metadata::HEIGHT]) {
      return FALSE;
    }
    if (isset($format) && $format != $still[mediamosa_asset_mediafile_metadata::STILL_FORMAT]) {
      return FALSE;
    }
    if (count($order) && !in_array($still[mediamosa_asset_mediafile_metadata::STILL_ORDER], $order)) {
      return FALSE;
    }

    return TRUE;
  }
}
