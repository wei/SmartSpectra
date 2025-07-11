package com.presagetech.smartspectra.utils

import android.content.Context
import com.presagetech.smartspectra.R

internal object PreferencesUtils {
    const val AGREED_TO_TERMS_OF_SERVICE_KEY = "agreed_to_terms_of_service"
    const val AGREED_TO_PRIVACY_POLICY_KEY = "agreed_to_privacy_policy"
    const val ONBOARDING_TUTORIAL_KEY = "onboarding_tutorial_has_been_shown"

    fun saveBoolean(context: Context, key: String, value: Boolean) {
        val sharedPreferences = context.applicationContext.getSharedPreferences(
            context.getString(R.string.shared_pref),
            Context.MODE_PRIVATE
        )
        sharedPreferences.edit().apply {
            putBoolean(key, value)
            apply()
        }
    }

    fun getBoolean(context: Context, key: String, defaultValue: Boolean): Boolean {
        val sharedPreferences = context.applicationContext.getSharedPreferences(
            context.getString(R.string.shared_pref),
            Context.MODE_PRIVATE
        )
        return sharedPreferences.getBoolean(key, defaultValue)
    }
}
