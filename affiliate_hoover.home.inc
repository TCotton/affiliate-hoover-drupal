<?php

/**
 * Define a form.
 * // make reference to where this function is being called
 */


function affiliate_hoover_first_form($form, &$form_state) {

    //variable_del('affiliate_hoover');

    $form['affiliate_hoover'] = array(
        '#title' => t('Add feed names'),
        '#type' => 'fieldset',
        '#description' => t('<p>Give your feed a name here.</p>'),
        );

    $form_data = variable_get('affiliate_hoover');

    static $i = 1;

    if ($form_data != NULL) {

        // reset array so first key has a value of 1
        $keys = range(1, count($form_data));
        $values = array_values($form_data);
        $form_data = array_combine($keys, $values);

        foreach ($form_data as $key => $value) {

            // loop through array to create the form
            
           $result = db_query('SELECT n.name FROM {ah_feed_details} n WHERE n.name = :name', array(':name' => $value))->fetchColumn();
           
           $html = ($result != '')? l('Edit feed form for ' . $value, 'admin/config/affiliate_hoover/uniqueform/' . $value) : null;
           
           $html .= '<br />';
           
           $html .= l('Upload feed file for ' . $value, 'admin/config/affiliate_hoover/uniquename/' . $value);
           
            
            $form['affiliate_hoover']['feed_name'][$key] = array(
                '#title' => t('Name of feed:'),
                '#type' => 'textfield',
                '#description' => t('Please enter a unique name for the feed.'),
                '#required' => true,
                '#default_value' => $value,
                '#prefix' => $html,
                '#disabled' => TRUE,
                );

            $form['affiliate_hoover']['feed_name']['checkbox'.$key] = array(
                '#title' => t('Delete entry'),
                '#type' => 'checkbox',
                '#description' => t('Delete above feed. This will delete all details about this feed from the database and the uploaded file, if any'),
                '#required' => false,
                '#default_value' => !empty($_REQUEST['feed_name']['checkbox'.$key]) ? $_REQUEST['feed_name']['checkbox'.$key] : '',
                );

        }

    }

    $form['affiliate_hoover']['feed_name']['0'] = array(
        '#title' => t('Name of feed:'),
        '#type' => 'textfield',
        '#description' => t('Please enter a unique name for the feed.'),
        '#required' => false,
        '#default_value' => !empty($_REQUEST['feed_name']['0']) ? check_plain(strip_tags($_REQUEST['feed_name']['0'])) : '',
        );

    $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'));

    return $form;

} // end affiliate_hoover_first_form



// find checkbox digit if set to true

function affiliate_hoover_find_checkbox_digit($form) {

    // If checkbox is set then find digit of the connected input field

    $pattern = '/^checkbox/';

    foreach ($form as $key => $value) {

        if (preg_match($pattern, $key) && $value == '1') {

            // remove entry from array of checkbox is set to true
            if (preg_match($pattern, $key)) {

                if ($value == '1') {

                    return preg_replace('/[^0-9]|,[0-9]*$/', '', $key);

                }

            } // end if if (preg_match($pattern, $key)) {

        } // end if (is_int($key) || preg_match($pattern, $key)) {

    } // end foreach

}

function affiliate_hoover_first_form_validate($form, &$form_state) {

    // check with database to make sure that name is unique

    $tmp = array();

    $digit = affiliate_hoover_find_checkbox_digit($form_state['values']);

    foreach ($form_state['values'] as $key => $value) {

        if (is_int($key)) {

            // if digit is set then remove the associated input from the array

            if (isset($digit)) {
                if ((int)$key == (int)$digit) continue;
            }

            if ($value == '') continue;

            $tmp[] = $value;

            if (!ctype_alnum($value)) {

                form_set_error('feed_name', 'Please don\'t include any special characters or spaces');
                break;

            } // end if if (!ctype_alnum($value)) {

        } // end if(is_int($key) || preg_match($pattern, $key)) {

    } // end foreach

    // this is a nice little trick to make sure that all values in the array are unique

    if (count($tmp) != count(array_unique($tmp))) {

        form_set_error('feed_name', 'Please make sure all feed names are unique');

    }

} // end affiliate_hoover_first_form_validate


function affiliate_hoover_first_form_submit($form, &$form_state) {

    $an_array = array();
    
    $digit = affiliate_hoover_find_checkbox_digit($form_state['values']);

    foreach ($form_state['values'] as $key => $value) {

        if (is_int($key)) {

            // if digit is set then remove the associated input from the array

            if (isset($digit)) {
                if ((int)$key == (int)$digit) continue;
            }

            if ($value == '') continue;

            $sanitized_data = stripslashes(trim($value));

            $an_array[] = $sanitized_data;

            variable_set('affiliate_hoover', $an_array);

        } // end if(is_int($key) || preg_match($pattern, $key)) {

    } // end foreach


    if (isset($sanitized_data)) {

        drupal_set_message(t('Returned data here: @return', array('@return' => $sanitized_data)));

    }

} // end affiliate_hoover_first_form_submit
