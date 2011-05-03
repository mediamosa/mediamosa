(function ($) {

Drupal.behaviors.initMediaMosaBrowse = {
  attach: function (context) {
    $('#edit-add-filter-row').click(function () {
      Drupal.settings.mediamosa_maintenance_browse.filter_count++;
      if (Drupal.settings.mediamosa_maintenance_browse.filter_count < Drupal.settings.mediamosa_maintenance_browse.filter_max) {
        $('#mediamosa-browse-asset-filter-' + (Drupal.settings.mediamosa_maintenance_browse.filter_count + 1)).slideDown(250);
        $('#mediamosa-browse-collection-filter-' + (Drupal.settings.mediamosa_maintenance_browse.filter_count + 1)).slideDown(250);
      }
      if ((Drupal.settings.mediamosa_maintenance_browse.filter_count + 1) == Drupal.settings.mediamosa_maintenance_browse.filter_max) {
        $('#edit-add-filter-row').attr('disabled', 'disabled');
      }
      return false;
    });

    $('#edit-clear').click(function () {
      // Hide all added filter items
      $('.mediamosa-browse-asset-filter:not(.first), .mediamosa-browse-collection-filter:not(.first)').slideUp(250);

      // Reset all form values
      $('#mediamosa-browse-asset-form input[type=text], #mediamosa-browse-collection-form input[type=text]').attr('value', '');
      $('#mediamosa-browse-asset-form select, #mediamosa-browse-collection-form select').attr('selectedIndex', 0);

      // Reset the 'add filter item' button
      $('#edit-add-filter-row').removeAttr('disabled');

      // Reset the filter count
      Drupal.settings.mediamosa_maintenance_browse.filter_count = 0;
      return false;
    });
  }
};

})(jQuery);
