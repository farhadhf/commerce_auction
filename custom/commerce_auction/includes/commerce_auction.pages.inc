<?php
/**
 * Forms for creating products and auctions.
 */

/**
 * Returns a suggestion based on the name of the product that this user is typing
 */
function commerce_auction_product_autocomplete($category, $name)
{
  $cat = taxonomy_term_load($category);
  
  // select from taxonomy index where tid = $cat2->tid, (and what about tags??)
  $nids = taxonomy_select_nodes($cat->tid, FALSE, 10);
  
  $items = array();
  if (!empty($nids))
  {
    foreach ($nids as $nid)
    {
      $node = node_load($nid);
      
      if (preg_match("/$name/i", $node->title))
        $items[$node->title] = $node->title;
    }
  }
  
  drupal_json_output($items);
}

/**
 * TODO: Specific form for each category of products.
 */
function commerce_auction_auction_creation_steps()
{
  return array(
    1 => array(
      // procut category
      'name' => t('Product information'),
      'form' => 'commerce_auction_product_info_form',
    ),
    2 => array(
      // image and video of the products?
      'name' => t('Product pictures'),
      'form' => 'commerce_auction_product_images_form',
    ),
    3 => array(
      'name' => t('Auction settings'),
      'form' => 'commerce_auction_settings_form',
    ),
  );
}

function commerce_auction_add_auction_wizard($form, &$form_state) {
  if (empty($form_state['step'])) {
    $form_state['step'] = 1;
    $form_state['step_information'] = commerce_auction_auction_creation_steps();
  }
  $step = &$form_state['step'];
  drupal_set_title(t('Auction creation - @step', array('@step' => $form_state['step_information'][$step]['name'])));

  $form = $form_state['step_information'][$step]['form']($form, $form_state);

  if ($step > 1) {
    $form['prev'] = array(
      '#type' => 'submit',
      '#value' => t('Previous'),
      '#name' => 'prev',
      '#submit' => array('form_wizard_previous_submit'),
      '#limit_validation_errors' => array(),
    );
  }

  // Show the Next button only if there are more steps defined.
  if ($step < count($form_state['step_information'])) {
    $form['next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
      '#name' => 'next',
      '#submit' => array('form_wizard_next_submit'),
    );
  }
  else {
    $form['finish'] = array(
      '#type' => 'submit',
      '#value' => t('Finish'),
    );
  }

  // Include each validation function defined for the different steps.
  if (function_exists($form_state['step_information'][$step]['form'] . '_validate')) {
    $form['next']['#validate'] = array($form_state['step_information'][$step]['form'] . '_validate');
  }

  return $form;
}

function commerce_auction_product_info_form($form, &$form_state) {
  //TODO: add default values from $form_state['values']
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'commerce_auction') . '/auction-form.css',
  );
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'commerce_auction') . '/auction-form.js',
  );
  
  $form['description'] = array( 
    '#type' => 'markup',
    '#markup' => '<div class="form-description">' . t('a description about this form goes here...') . '</div>',
  );
  
  $vocab = taxonomy_vocabulary_machine_name_load('products');
  if (!$vocab) {
    return NULL;
  }
  
  $form['category'] = array(
    '#type' => 'fieldset',
    '#title' => t('Category'),
    '#description' => t('Select the category of your product, this will help other users find your product more easily'),
  );
  
  $children = taxonomy_get_tree($vocab->vid);
  $tree = taxonomy_get_tree($vocab->vid);
  $level0 = array();
  foreach ($tree as $item) {
    if ($item->depth == 0) {
      $level0[$item->tid] = $item->name;
    }
  }
  
  $form['category']['cat0'] = array(
    '#type' => 'select',
    '#options' => $level0,
    '#prefix' => '<table class="invisible-table"><tr><td>',
    '#suffix' => '</td><td>',
    '#required' => true,
    '#ajax' => array(
      'wrapper' => 'cat2',
      'callback' => 'commerce_auction_category_2_callback',
      'effect' => 'fade',
      'progress' => array('type' => 'none'),
    )
  );
  $options = array();
  if (!empty($form_state['values']['cat0'])) {
    $tree = taxonomy_get_tree($vocab->vid, $form_state['values']['cat0']);
    $options = array();
    foreach ($tree as $item) {
      $options[$item->tid] = str_repeat('-', $item->depth) . $item->name;
    }
  }
  $form['category']['cat1'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#required' => true,
    '#prefix' => '<div id="cat2">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'product-name-wrapper',
      'callback' => 'commerce_auction_product_name_callback',
      'effect' => 'fade',
      'progress' => array('type' => 'none'),
    )
  );
  $form['category']['tablecloser'] = array(
    '#type' => 'markup',
    '#markup' => '</td></tr></table>'
  );
  $autocomplete_path = FALSE;
  if (!empty($form_state['values']['cat1']))
  {
    // first and second level categories are passed to autocomplete callback as arguments, so we can narrow our response 
    // having product category in mind!
    $autocomplete_path = 'products/autocomplete/' . $form_state['values']['cat1'];
  }
  
  $form['product_wrapper'] = array(
    '#title' => t('General product information'),
    '#type' => 'fieldset',
  );
  $form['product_wrapper']['product_name_wrapper'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="product-name-wrapper">',
    '#suffix' => '</div>',
  );
  $form['product_wrapper']['product_name_wrapper']['product_name'] = array(
    '#title' => t('Product name'),
    '#type' => 'textfield',
    '#required' => true,
    '#ajax' => array(
      'wrapper' => 'product-info-wrapper',
      'callback' => 'commerce_auction_product_info_fieldset_callback',
      'effect' => 'fade',
      'progress' => array('type' => 'none'),
    ),
  );
  if ($autocomplete_path) {
    $form['product_wrapper']['product_name_wrapper']['product_name']['#autocomplete_path'] = $autocomplete_path;
  }
  
  $form['product_wrapper']['info'] = array(
    '#type' => 'markup',
    '#markup' => '',
    '#prefix' => '<div id="product-info-wrapper">',
    '#suffix' => '</div>',
  );
  $form['product_wrapper']['info']['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#required' => true,
    '#description' => t('Write about your product. Specifications and description of the product should be here.'),
  );
  
  if (!empty($form_state['values']['product_name']))
  {
    
    // product_name is title of a node, so I'll have to select the node via its name, uglyyy!!!!
    $nid = db_select('node', 'n')->condition('title', $form_state['values']['product_name'], '=')->fields('n', array('nid'))->execute()->fetchField();
    
    $node = node_load($nid);
    if ($node != NULL) {
      $body = field_get_items('node', $node, 'body');
    }
    
    // select product object, load attach its field form to this form.
    $form['product_wrapper']['info']['description']['#value'] = $body[0]['value'];
    $form_state['#matched_node'] = $node;
  }
  
  return $form;
}

function commerce_auction_product_images_form($form, &$form_state) {
  $node = $form_state['#matched_node'];
  $field_name = find_field_name('node', $node->type, 'commerce_product_reference');
  $product_ref = $node->{$field_name}[LANGUAGE_NONE];
  
  $product = commerce_product_load($product_ref[0]['product_id']);
  $field_name = find_field_name('commerce_product', $product->type, 'image');
  
  $images = array();
  foreach ($product->{$field_name}[LANGUAGE_NONE] as $delta => $image) {
    $url = file_create_url($image['uri']);
    $images[$delta] = $url;
  }
  if (!empty($images)) {
    $form['description'] = array(
      '#type' => 'markup',
      '#markup' => '<p>' . t('If you see an image that can be used for your product in these images, select it! Otherwise, upload yours!') . '</p>',
    );
    $form['suggested_images'] = array(
      '#type' => 'fieldset',
      '#title' => t('Suggested images'),
      '#description' => t('These images has been used for products with the same name as your product, It\'s possible that you can use them too!'),
    );
    foreach ($images as $delta => $url) {
      $form['suggested_images']['img-' . $delta] = array(
        '#type' => 'checkbox',
        '#title' => theme('image', array('path' => $url, 'width' => 200)),
      );
    }
    
  }
  else {
    $form['description'] = array(
      '#type' => 'markup',
      '#markup' => '<p>'. t('Upload a few images of your product') . '</p>',
    );
  }
  if (!isset($form_state['product_image_count'])) {
    $form_state['product_image_count'] = 3;
  }
  else if ($form_state['values']['more-images']) {
    $form_state['product_image_count']++;
  }
  $form['upload'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upload new images'),
  );
  for ($i = 1 ; $i<=$form_state['product_image_count'] ; $i++) {
    $form['upload']['new-image-' . $i] = array(
      '#type' => 'file',
      '#title' => '',
      '#size' => 40,
      '#value' => isset($form_state['values']['new-image-'.$i]) ? $form_state['values']['new-image-'.$i] : NULL,
    );
  }
  $form['upload']['next-image-wrapper'] = array(
    '#type' => 'markup',
    '#markup' => '<div id="next-image-wrapper"></div>',
  );
  $form['upload']['more-images'] = array(
    '#type' => 'submit',
    '#value' => t('Add more'),
    '#ajax' => array(
      'callback' => 'commerce_auction_add_another_image',
      'wrapper' => 'next-image-wrapper',
      'progress' => 'none',
      'effect' => 'fade',
      'method' => 'append',
    ),
  );
  
  return $form;
}

function commerce_auction_add_another_image($form, &$form_state) {
  return $form['upload']['new-image-' . $form_state['product_image_count']];
}

function commerce_auction_settings_form($form, &$form_state) {
  $form['#attached']['css'][] = drupal_get_path('module', 'timeout') . '/timeout_field.css';
  $form['auction_info'] = array(
    '#title' => t('Auction settings'),
    '#type' => 'fieldset',
  );
  
  $form['auction_info']['price'] = array(
    '#title' => t('Base price'),
    '#size' => 6,
    '#description' => t('Enter base price for this auction in tomans'),
    '#type' => 'textfield',
    '#field_suffix' => commerce_default_currency(),
    '#required' => true,
  );
  $form['auction_info']['timeout'] = array(
    '#title' => t('Finalization time'),
    '#type' => 'fieldset',
    '#description' => t('How much time do you want this auction to be open for bidding?'),
  );
  foreach (array('timout_d' => t('Days'), 'timeout_h' => t('Hours'), 'timeout_m' => t('Minutes')) as $key => $title) {
    $form['auction_info']['timeout'][$key] = array(
      '#type' => 'textfield',
      '#description' => $title,
      '#size' => 2,
      '#prefix' => '<div class="timeout-component-form-item">',
      '#suffix' => '</div>',
    );
  }
  
  $form['shipping'] = array(
    '#title' => t('Supported shipping methods'),
    '#description' => t('Select the shipping methods that you support for this product.'),
    '#type' => 'fieldset',
  );
  
  global $user;
  $hometown_field_name = commerce_iran_post_city_field_name('user');
  $hometown = NULL;
  $shipper_province = NULL;
  $shipper_city = NULL;
  if (empty($hometown_field_name))
  {
    drupal_set_message(t('There is a configuration problem, user has no hometown field attached to it. Please contact site administrator.'), 'error');
  }
  else {
    $account = user_load($user->uid);
    $hometown = field_get_items('user', $account, $hometown_field_name);
    if (empty($hometown)) {
      drupal_set_message(t('Seller Hometown is not set, You will not see this form completely, please set your hometown and try again to see all shipping options.'), 'warning');
    }
    else {
      $shipper_province = $hometown[0]['province'];
      $shipper_city = $hometown[0]['city'];
    }
  }
  $options = array(
    'free-shipping-country' => t("I'll provide free shipping to all over Iran"),
  );
  $provinces = iran_provinces();
  if ($shipper_city) {
    $options['free-shipping-city'] = t("I'll provide free shipping to @city city", array('@city' => $shipper_city));
  }
  if ($shipper_province) {
    $options['free-shipping-province'] = t("I'll provide free shipping to @province state", array('@province' => $provinces[$shipper_province]));
  }
  $options['nonfree-shipping'] = t("I won't provide free shipping");
  $form['shipping']['shipping_methods'] = array(
    '#type' => 'radios',
    '#options' => $options,
    '#ajax' => array(
      'wrapper' => 'shipping-method-preferences-wrapper',
      'callback' => 'commerce_auction_shipping_preferences_fieldset_callback',
      'effect' => 'fade',
      'progress' => array('type' => 'none'),
    ),
  );
  $form['shipping']['preferences_wrapper'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="shipping-method-preferences-wrapper">',
    '#suffix' => '</div>',
  );
  
  if (isset ($form_state['values']['shipping_methods']) && in_array($form_state['values']['shipping_methods'],array('nonfree-shipping', 'free-shipping-province'))) {
    $form['shipping']['preferences_wrapper']['nonfree_methods'] = array(
      '#title' => t('Shipping preferences'),
      '#type' => 'fieldset',
      '#description' => t('Select the shipping methods that you will support'),
    );
    $form['shipping']['preferences_wrapper']['nonfree_methods']['post'] = array(
      '#title' => t('Iran post service'),
      '#description' => t('You will send the product to the buyer by either Pishtaz or Sefareshi post, We WILL CHARGE the shipping fee on checkout and give it to you when the product is sent.'),
      '#type' => 'checkbox',
    );
    $form['shipping']['preferences_wrapper']['nonfree_methods']['peik'] = array(
      '#title' => t('Peik or Ajans'),
      '#description' => t('You will send the product to the buyer via peik or ajans. Buyer pays the shipping fee when she receives the product. We WILL NOT CHARGE the buyer for this type of shipping. This shipping method will only be availabe if the buyer is in the same city as you are.'),
      '#type' => 'checkbox',
    );
    $form['shipping']['preferences_wrapper']['nonfree_methods']['other'] = array(
      '#title' => t('Other'),
      '#description' => t('You want to send the product with a different shipping method - for example you ship the product yourself and maybe install it for the buyer. We WILL NOT CHARGE the buyer for this kind of shipping method, you have to do it yourself.'),
      '#type' => 'checkbox',
    );
  }
  else if (isset ($form_state['values']['shipping_methods']) && $form_state['values']['shipping_methods'] == 'free-shipping-city') {
    $form['shipping']['preferences_wrapper']['nonfree_methods'] = array(
      '#title' => t('Shipping preferences'),
      '#type' => 'fieldset',
      '#description' => t('Select the shipping methods that you will support'),
    );
    $form['shipping']['preferences_wrapper']['nonfree_methods']['post'] = array(
      '#title' => t('Iran post service'),
      '#description' => t('You will send the product to the buyer by either Pishtaz or Sefareshi post, We WILL CHARGE the shipping fee on checkout and give it to you when the product is sent.'),
      '#type' => 'checkbox',
    );
    $form['shipping']['preferences_wrapper']['nonfree_methods']['other'] = array(
      '#title' => t('Other'),
      '#description' => t('You want to send the product with a different shipping method - for example you ship the product yourself and maybe install it for the buyer. We WILL NOT CHARGE the buyer for this kind of shipping method, you have to do it yourself.'),
      '#type' => 'checkbox',
    );
  }
  
  return $form;
}

/**
 * validate selected categories.
 * attacker may post invalid categories, potentially texts containing php codes, SQL commands or
 * codes used for xss attacks.
 */
function commerce_auction_add_auction_form_validate($form, &$form_state)
{
  // first check to see if the text is safe! (does not include php/sql code)
  if (!is_numeric($form_state['values']['first_level_category']))
  {
    form_set_error('first_level_category', t('Select a category from the given list'));
  }
  if (!is_numeric($form_state['values']['second_level_category']))
  {
    form_set_error('second_level_category', t('Select a category from the given list'));
  }

  // now check to see if term hierarchy is preserved or the user has posted some bullshit of his own.
  $parent_term = taxonomy_term_load($form_state['values']['first_level_category']);
  $terms = taxonomy_get_tree($parent_term->vid);
  $child_term = NULL;
  foreach ($terms as $term)
  {
    if ($term->tid == $form_state['values']['second_level_category'])
    {
      $child_term = $term;
      break;
    }
  }
  
  if ($child_term->parents[0] != $form_state['values']['first_level_category'])
  {
    form_set_error('second_level_category', t('Select a category from the given list'));
  }
  
  $form_state['values']['product_name'] = filter_xss($form_state['values']['product_name']);
  $form_state['values']['description'] = filter_xss($form_state['values']['description']);
  
  if (!is_numeric($form_state['values']['timeout_days']) || $form_state['values']['timeout_days'] < 0 ) {
    form_set_error('timeout_days', t('Enter a non-negative numeric value for auction timeout day period.'));
  }
  if (!is_numeric($form_state['values']['timeout_hours']) || $form_state['values']['timeout_hours'] < 0 ) {
    form_set_error('timeout_hours', t('Enter a non-negative numeric value for auction timeout hour period.'));
  }
  if (!is_numeric($form_state['values']['timeout_minutes']) || $form_state['values']['timeout_minutes'] < 0 ) {
    form_set_error('timeout_minutes', t('Enter a non-negative numeric value for auction timeout minute period.'));
  }
  
  if ($form_state['values']['timeout_days'] == 0 && $form_state['values']['timeout_hours'] == 0 && $form_state['values']['timeout_minutes'] == 0) {
    form_set_error('timeout_days', t('Auction timeout period can not be zero.'));
  }
  
  
  if ($form_state['values']['free_shipping'] == 0) {
    // seller is going to provide free shipping service so she shouldn't select nonfree shipping methods too.
    if (
        (isset($form_state['values']['post']) && $form_state['values']['post']) ||
        (isset($form_state['values']['peik']) && $form_state['values']['peik']) ||
        (isset($form_state['values']['other']) && $form_state['values']['other'])
       )
    {
      form_set_error('free_shipping', t('Free shipping and paid shipping methods should not be selected together.'));
    }
  }
  else {
    if (!$form_state['values']['post'] && !$form_state['values']['peik'] && !$form_state['values']['other']) {
      form_set_error('free_shipping', t('You should enable at least one paid shipping method.'));
    }
  }
}

function commerce_auction_add_auction_form_submit($form, &$form_state)
{
  dsm($form_state);
  
}

function commerce_auction_product_info_fieldset_callback($form, &$form_state) {
  return $form['product_wrapper']['info'];
}


function commerce_auction_product_name_callback($form, &$form_state)
{
  return $form['product_wrapper']['product_name_wrapper'];
}

function commerce_auction_shipping_preferences_fieldset_callback($form, &$form_state) {
  return $form['shipping']['preferences_wrapper'];
}

function form_wizard_previous_submit($form, &$form_state) {
  $current_step = &$form_state['step'];
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];
  if ($current_step > 1) {
    $current_step--;
    $form_state['values'] = $form_state['step_information'][$current_step]['stored_values'];
  }
  dsm($form_state);
  $form_state['rebuild'] = TRUE;
}

function form_wizard_next_submit($form, &$form_state) {
  $current_step = &$form_state['step'];
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];

  if ($current_step < count($form_state['step_information'])) {
    $current_step++;
    if (!empty($form_state['step_information'][$current_step]['stored_values'])) {
      $form_state['values'] = $form_state['step_information'][$current_step]['stored_values'];
    }
    else {
      $form_state['values'] = array();
    }
    $form_state['rebuild'] = TRUE;  // Force rebuild with next step.
    return;
  }
}


function commerce_auction_generate_taxonomy_tree(&$tree = array()) {
  foreach(array_keys($tree) as $key) {
    $term = taxonomy_term_load($key);
    foreach (taxonomy_get_children($key) as $t) {
      $tree[$key][$t->tid] = array(); 
    }
    if (!empty($tree[$key]))
      commerce_auction_generate_taxonomy_tree($tree[$key]);
  }
}

function commerce_auction_process_taxonomy_tree_select(&$tree = array()) {
  foreach (array_keys($tree) as $tid) {
    $term = taxonomy_term_load($tid);
    if (empty($tree[$tid])) {
      $tree[$tid] = $term->name;
    }
    else {
      $tree[$term->name] = $tree[$tid];
      unset($tree[$tid]);
      commerce_auction_process_taxonomy_tree_select($tree[$term->name]);
    }
  }
}

function commerce_auction_category_2_callback($form, &$form_state) {
  return $form['category']['cat1'];
}

function find_field_name($entity_type, $bundle, $field_type) {
  foreach (field_info_instances($entity_type, $bundle) as $instance_name => $instance) {
    // Load the field info for the current instance.
    $field = field_info_field($instance['field_name']);

    // If it's of the proper type...
    if ($field['type'] == $field_type) {
      // Save its name and exit the loop if it isn't the billing profile.
      return $instance_name;
    }
  }
  return NULL;
}