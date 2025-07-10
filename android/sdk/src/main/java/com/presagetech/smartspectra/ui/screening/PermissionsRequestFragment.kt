package com.presagetech.smartspectra.ui.screening

import android.Manifest
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.Settings
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.result.contract.ActivityResultContracts
import androidx.fragment.app.Fragment
import com.presagetech.smartspectra.R

internal class PermissionsRequestFragment: Fragment() {

    private lateinit var requestButton: View
    private lateinit var settingsButton: View

    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { isGranted: Boolean ->
        if (isGranted) {
            toggleButtons(true)
        } else {
            toggleButtons(false)
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragment_permissions_layout, container, false).also {
            requestButton = it.findViewById(R.id.button_allow)
            settingsButton = it.findViewById(R.id.button_open_settings)
        }

        requestButton.setOnClickListener {
            requestPermissionDialog()
        }
        settingsButton.setOnClickListener {
            openPermissionsSettings()
        }

        toggleButtons(true)
        return view
    }

    private fun toggleButtons(canRequest: Boolean) {
        requestButton.visibility = if (canRequest) View.VISIBLE else View.GONE
        settingsButton.visibility = if (canRequest) View.GONE else View.VISIBLE
    }

    private fun requestPermissionDialog() {
        requestPermissionLauncher.launch(Manifest.permission.CAMERA)
    }

    private fun openPermissionsSettings() {
        val intent = Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS)
        val uri = Uri.fromParts("package", requireContext().packageName, null)
        intent.data = uri
        requireContext().startActivity(intent)
    }
}
