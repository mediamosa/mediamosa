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
 * PDF tool functions.
 */
class mediamosa_tool_pdf {

/**
 * Mime type: application/pdf
 */
  const MEDIAMOSA_TOOL_PDF_MIME_TYPE_PDF = 'application/pdf';

  /**
   * Short id of the module.
   */
  const NAME = 'pdf';

  /**
   * Key value separator.
   */
  const MEDIAMOSA_TOOL_PDF_KEY_VALUE_SEPARATOR = ':';

  /**
   * Warning level for pdf2swf.
   */
  const MEDIAMOSA_MAINTENANCE_STATUS_CONFIGURATION_PDF2SWF_WARNING = '0';

  /**
   * Warning level for pdfinfo.
   */
  const MEDIAMOSA_MAINTENANCE_STATUS_CONFIGURATION_PDFINFO_WARNING = '0';

  /**
   * Is this my supported mime type.
   *
   * @param string $mime_type
   */
  public static function is_supported($mime_type) {
    return in_array($mime_type, array(self::MEDIAMOSA_TOOL_PDF_MIME_TYPE_PDF));
  }

  /**
   * Get the exec string for transcoding the file.
   *
   * @deprecated: Although new, its deprecated. We will replace this function
   *              with transcode() very soon and no longer rely on exec
   *              strings return. Rewrite of jobs is planned for v3.5.
   *
   * @param array $options
   */
  public static function get_transcode_exec($options) {

    if ($options['file_extension'] == 'swf') {
      // Build the values.
      $execution_string_options = array(
        '@file_location' => dirname($options['location_dest_file']) . DIRECTORY_SEPARATOR . $options['jobserver_job_id'] . DIRECTORY_SEPARATOR,
        '@mediafile_location' => $options['location_source_file'],
        '@mediafile_id' => $options['mediafile_id'],
        '@params' => $options['parameter_string'],
      );

      // Build the execute_string.
      return strtr('mkdir @file_location; cd @file_location; pdf2swf @mediafile_location -o @mediafile_id @params; cp @mediafile_id ../@mediafile_id.swf; cd ..; rm -r @file_location; { echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @mediafile_id.status;', $execution_string_options);
    }

    return FALSE;
  }
}
