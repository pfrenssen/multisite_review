<?php
/**
 * hook_form()
 * @param $form
 * @param $form_state
 * @return mixed
 */
function multisite_review_form($form, &$form_state) {
    $form['what_to_review'] = array(
        '#type' => 'textfield',
        '#description' => 'Fill in with the feature/module to be reviewed or leave it empty to review all custom',
        '#title' => t('Fill in the feature to review'),
        '#default_value' => variable_get('what_to_review', 'what_to_review'),
    );

    return system_settings_form($form);
}

