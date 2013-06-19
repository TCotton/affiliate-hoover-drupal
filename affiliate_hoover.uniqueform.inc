<?php

function affiliate_hoover_in_array_like($referencia, $array = array()) {

    foreach ($array as $ref) {
        if (stristr($referencia, $ref)) {
            return true;
        }
    }

    return false;

}


// checks that URL item is actual form. returns false if not and is used in a 404 message below
function affiliate_hoover_unique_form($args) {

    $aff = db_query('SELECT n.name FROM {ah_feed_details} n WHERE n.name = :name',
        array(':name' => arg(4)))->fetchAssoc();

    if ($aff) {

        $result = affiliate_hoover_in_array_like($args, $aff);

    } else {

        $result = FALSE;

    }

    if (!$result || !$aff) {

        return drupal_not_found();
        drupal_exit();

    } else {

        return drupal_get_form('affiliate_hoover_uniqueform_build_form');

    } // end if

} // end if affiliate_hoover_unique_name()

function affiliate_hoover_uniqueform_build_form($form, &$form_state) {

    $form_data = db_query('SELECT n.* FROM {ah_feed_details} n WHERE n.name = :name',
        array(':name' => arg(4)))->fetchObject();


    // find available not types
    $node_types = node_type_get_types();

    $total_types = array();

    foreach ($node_types as $key => $value) {

        $total_types[$key] = $key;

    }

    //print '<pre>';
    //var_dump(implode(',',$total_types));
    //var_dump($total_types);
    //print '</pre>';

    $html = "";

    foreach (unserialize($form_data->header_array_amend) as $key => $value) {
        $html .= $key . ' = ' . $value . '<br />';

    }

    $form['affiliate_hoover_unique_form'] = array('#type' => 'item',
        //'#title' => t('Feed details for ' . arg(4)),
        '#description' => 'Some description here', '#prefix' => $html, );

    $form['first_part'] = array('#type' => 'fieldset', '#title' => t('Feed details for ' .
        arg(4)), '#description' => '', );

    $form['first_part']['form_title'] = array('#type' => 'textfield', '#title' => t
        ('Title'), '#required' => TRUE, '#maxlength' => 250, '#description' => t('Do not add anything other than the above codes for a title'),
        '#default_value' => !empty($_REQUEST['form_title']) ? $_REQUEST['form_title']:$form_data->
        form_title, );

    $form['first_part']['form_body'] = array('#type' => 'textarea', '#title' => t('Post body'),
        '#required' => TRUE, '#description' =>
        'You can use HTML in here. Examples:<br>To place an image: ' . htmlspecialchars
        ("<img src=\"[#7#]\">") . "<br>To create a link: " . htmlspecialchars("<a href=\"[#5#]\">[#1#]</a>") .
        '<br>If you are going to include internal links you must write the full URL, ie ' .
        htmlspecialchars("http://www.example.com/category/page-here"), '#default_value' =>
        !empty($_REQUEST['form_body']) ? $_REQUEST['form_body']:$form_data->form_body, );

    $form['first_part']['form_nofollow'] = array('#type' => 'checkbox', '#title' =>
        t('Nofollow'), '#description' => t('Turn links in the body text into nofollow'),
        '#default_value' => !empty($_REQUEST['form_nofollow']) ? $_REQUEST['form_nofollow']:
        $form_data->form_body_nofollow, );

    $form['first_part']['form_taxonomy'] = array('#type' => 'textfield', '#title' =>
        t('Taxonomy'), '#maxlength' => 250, '#description' => t('Can be either text or code. All values must be separated with a comma'),
        '#default_value' => !empty($_REQUEST['form_taxonomy']) ? $_REQUEST['form_taxonomy']:
        $form_data->form_categories, );

    $form['first_part']['form_comment'] = array('#type' => 'checkbox', '#title' => t
        ('Comment'), '#description' => t('Whether comments are allowed on this node'),
        '#default_value' => !empty($_REQUEST['form_comment']) ? $_REQUEST['form_comment']:
        $form_data->form_allow_comments, );

    $form['first_part']['form_promote'] = array('#type' => 'checkbox', '#title' => t
        ('Promote'), '#description' => t('Indicating whether the node should be displayed on the front page.'),
        '#default_value' => !empty($_REQUEST['form_promote']) ? $_REQUEST['form_promote']:
        $form_data->form_promote, );

    $form['first_part']['content_type'] = array('#type' => 'select', '#title' => t('Content type'),
        '#description' => t('What content type should this feed be allocated to?'),
        '#default_value' => !empty($_REQUEST['content_type']) ? $_REQUEST['content_type']:
        $form_data->post_type, '#options' => $total_types, );

    $form['first_part']['form_minrows'] = array('#type' => 'textfield', '#title' =>
        t('Start on this row'), '#maxlength' => 11, '#description' => t('Start processing on which row? (Out of a total of ' .
        $form_data->num_rows . ' entries)'), '#default_value' => !empty($_REQUEST['form_minrows']) ?
        $_REQUEST['form_minrows']:$form_data->min_rows, );

    $form['first_part']['form_maxrows'] = array('#type' => 'textfield', '#title' =>
        t('End on this row'), '#maxlength' => 11, '#description' => t('End processing on which row? (Out of a total of ' .
        $form_data->num_rows . ' entries)'), '#default_value' => !empty($_REQUEST['form_maxrows']) ?
        $_REQUEST['form_maxrows']:$form_data->max_rows, );

    $form['submit'] = array('#type' => 'submit', '#value' => 'Submit', );

    return $form;

}


/**
 * Validation for unmanaged file form.
 */
function affiliate_hoover_uniqueform_build_form_validate($form, &$form_state) {

    //print '<pre>';
    //var_dump($form_state);
    //print '</pre>';

    if (!preg_match("/^\[#([0-9]*)#\]$/", trim($form_state['values']['form_title']))) {

        form_set_error('form_title', t('Only include one code such as [#3#] and nothing else for the title. You can change the title once the form has been created.'));

    }


    if ((isset($form_state['values']['form_minrows']) && !isset($form_state['values']['form_maxrows'])) ||
        (!isset($form_state['values']['form_minrows']) && isset($form_state['values']['form_maxrows']))) {

        form_set_error('form_minrows', t('Please make sure that set both a min rows number and a max rows number'));

    }

}


/**
 * Submit function for unmanaged file form.
 */
function affiliate_hoover_uniqueform_build_form_submit($form, &$form_state) {

    affiliate_hoover_update_ah_feed_details($form_state);

    affiliate_hoover_process_form($form_state);

    drupal_set_message(t("Your form has been updated"));

}

function affiliate_hoover_update_ah_feed_details($form_state) {

    $update = db_update('ah_feed_details')->fields(array('form_title' => $form_state['values']['form_title'],
        'form_body' => $form_state['values']['form_body'], 'form_body_nofollow' => $form_state['values']['form_nofollow'],
        'form_categories' => $form_state['values']['form_taxonomy'],
        'form_allow_comments' => $form_state['values']['form_comment'], 'form_promote' =>
        $form_state['values']['form_promote'], 'post_type' => $form_state['values']['content_type'],
        'min_rows' => $form_state['values']['form_minrows'], 'max_rows' => $form_state['values']['form_maxrows'], ))->
        condition('name', arg(4))->execute();

    return $update;


}

function affiliate_hoover_process_form() {

    $item = db_query('SELECT n.* FROM {ah_feed_details} n WHERE n.name = :name',
        array(':name' => arg(4)))->fetchObject();

    $headers = unserialize($item->header_array);

    global $user;

    // $user->uid

    $newNode = (object)NULL;
    $newNode->type = 'node_example';
    $newNode->uid = $user->uid;
    $newNode->created = strtotime("now");
    //$newNode->changed = strtotime("now");
    $newNode->status = 1;
    $newNode->comment = 0;
    $newNode->promote = 0;
    $newNode->moderate = 0;
    $newNode->sticky = 0;
    $newNode->language = 'und';


    $csv = new File_CSV_DataSource;

    if ($csv->load($item->url)) {

        $csv->symmetrize();

        $total = new ArrayIterator($csv->getrawArray());

        foreach ($total as $result => $value) {

            if ($item->min_rows != FALSE && $item->max_rows != FALSE) {

                if ($result > $item->max_rows) continue;
                if ($result < $item->min_rows) continue;

            }


            if ($result == 0) continue;

            $total_val = count($value);

            foreach ($value as $key => $row_value) {

                if ($key === 0) {

                    $post_title = $item->form_title;
                    $post_content = $item->form_body;
                    $post_category = $item->form_categories;
                    $post_cat_array = array();
                    $post_type = $item->post_type;

                } // end if ($key === 0) {

                // Post title

                if (stristr($item->form_title, "[#$key#]") !== FALSE) {

                    $post_title = str_replace("[#$key#]", $row_value, $post_title);

                }

                // Post content

                if (stristr($item->form_body, "[#$key#]") !== FALSE) {

                    $post_content = str_replace("[#$key#]", $row_value, $post_content);

                }

              

                // form cats

                if (stristr($item->form_categories, "[#$key#]") !== FALSE) {

                    $post_category = str_replace("[#$key#]", $row_value, $post_category);

                }

                // Allow comments

                if ($item->form_allow_comments == TRUE) {
                    $comment_status = 'open';
                } else {
                    $comment_status = 'closed';
                }

                if ($item->form_body_nofollow == TRUE) {
                    $nofollow = TRUE;
                } else {
                    $nofollow = FALSE;
                }


                static $i = 1;
                static $x = 1;

                if ($key === ($total_val - 1)) {

                    $newNode->title = stripslashes(affiliate_hoover_check_utf8($post_title));

                    if ($nofollow) {

                        $body = stripslashes(affiliate_hoover_dont_follow_links(affiliate_hoover_add_tracking_link
                            (affiliate_hoover_check_utf8($post_content))));

                    } else {

                        $body = stripslashes(affiliate_hoover_add_tracking_link(affiliate_hoover_check_utf8
                            ($post_content)));

                    }

                    $newNode->body[$language][0] = array('value' => $body, 'format' => 'full_html', );

                    $newNode->comment = $comment_status;
                    $newNode->type = $post_type;

                    $duplicate = FALSE;
                    
                     foreach ($post_meta as $result) {
                        
                        
                        
                        
                        
                        
                        }// end foreach ($post_meta as $result) {
                    
                    /*
                    ["values"]=>
  array(14) {
    ["form_title"]=>
    string(5) "[#9#]"
    ["form_body"]=>
    string(0) ""
    ["form_nofollow"]=>
    int(1)
    ["form_taxonomy"]=>
    string(6) "[#15#]"
    ["form_comment"]=>
    int(1)
    ["form_promote"]=>
    int(1)
    ["content_type"]=>
    string(4) "page"
    ["form_minrows"]=>
    string(2) "50"
    ["form_maxrows"]=>
    string(2) "60"
    ["submit"]=>
    string(6) "Submit"
    ["form_build_id"]=>
    string(48) "form-Zo4CX6J5HRP6jUvgX1wqBzKcmNgvzRxmPsU9w5XjRdQ"
    ["form_token"]=>
    string(43) "Gx69AlL3odaVt9hf7162k0APDg_Xhsy56eatq9FCQnM"
    ["form_id"]=>
    string(38) "affiliate_hoover_uniqueform_build_form"
    ["op"]=>
    string(6) "Submit"
  }
                    
                    */

                   


                

                } // end  if ($key === ($total_val - 1)) {

            } // end  foreach ($value as $key => $row_value) {

        } // end foreach ($total as $result => $value) {

    } // end if ($csv->load(AH_FEEDS_DIR.$file_here)) {

}


/**
 * Implements hook_file_validate().
 *
 * We're testing to see if the string 'test.' exists in the file name.
 */
/*
function affiliate_hoover_uniquename_build_form_file_validate($file, $string = 'test.') {
$errors = array();
if (strpos($file->filename, $string) !== FALSE) {
$errors[] = t('The string "%string" is reserved, please remove this string from your filename and try again.',
array('%string' => $string));
}
return $errors;
}
*/


//http://stackoverflow.com/a/3257169/315350
// function adds nofollow to links
function affiliate_hoover_dont_follow_links($html) {
    // follow these websites only!
    $follow_list = array($_SERVER['HTTP_HOST']);
    return preg_replace('%(<a\s*(?!.*\brel=)[^>]*)(href="https?://)((?!(?:(?:www\.)?' .
        implode('|(?:www\.)?', $follow_list) . '))[^"]+)"((?!.*\brel=)[^>]*)(?:[^>]*)>%',
        '$1$2$3"$4 rel="nofollow">', $html);
}

// adds class of ah_link to outbound links,
// This is intended as for use with tracking clicks on outbound links
function affiliate_hoover_add_tracking_link($html) {
    // follow these websites only!
    $follow_list = array($_SERVER['HTTP_HOST']);
    $out = preg_replace('/(<a[^>]*?)(class\s*\=\s*\"[^\"]*?\")([^>]*?>)/', '$1$3', $html);
    return preg_replace('%(<a\s*(?!.*\brel=)[^>]*)(href="https?://)((?!(?:(?:www.)?' .
        implode('|(?:www\.)?', $follow_list) . '))[^"]+)"((?!.*\brel=)[^>]*)(?:[^>]*)>%',
        '$1$2$3"$4 class="ah_link ">', $out);

}

function affiliate_hoover_get_post_meta() {

          return $wpdb->get_results("SELECT post_id, meta_value FROM ".$wpdb->prefix.
            "postmeta WHERE meta_key = '_unique_post'");
            
    }


function affiliate_hoover_check_utf8($str) {

    if (mb_detect_encoding($str, 'UTF-8', TRUE) == FALSE) {
        return utf8_encode($str);
    } else {
        return $str;
    }

}

function affiliate_hoover_unique_form_title_callback($args) {

    //var_dump($args);
    return t($args);

}
