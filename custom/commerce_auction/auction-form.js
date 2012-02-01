/*
 * @file ajax_example.js
 *   JavaScript for ajax_example.
 *
 * See @link ajax_example_dependent_dropdown_degrades @endlink for
 * details on what this file does. It is not used in any other example.
 * 
 */

(function($) {

  // Re-enable form elements that are disabled for non-ajax situations.
  Drupal.behaviors.enableFormItemsForAjaxForms = {
    attach: function() {
      var first_level = jQuery('select[name="first_level_category"]').val();
      if (!first_level)
        jQuery(':input[name="second_level_category"]').html('<option value="-1">-Select-</option>');
      jQuery('select[name="first_level_category"]').change(
        function () {
          var selected = this.value;
          var second_level = jQuery(':input[name="second_level_category"]');
          opts = '<option value="-1">---</option>';
          jQuery.each(
            Drupal.settings.taxonomy_tree[selected]['children'],
            function(index, term) {
              opts += '<option value="' + index + ((index == -1) ? '" selected' : '"') + '>' + term.name + "</option>";
            }
          );
          second_level.html(opts);
        }
      );
    }
  };

})(jQuery);
