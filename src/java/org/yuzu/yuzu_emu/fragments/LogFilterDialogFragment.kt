// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

package org.yuzu.yuzu_emu.fragments

import android.app.Dialog
import android.content.Context
import android.os.Bundle
import androidx.fragment.app.DialogFragment
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import org.yuzu.yuzu_emu.R

class LogFilterDialogFragment : DialogFragment() {
    
    interface LogFilterListener {
        fun onFilterLogs(filter: Boolean)
    }
    
    private var listener: LogFilterListener? = null
    
    companion object {
        const val TAG = "LogFilterDialogFragment"
        
        fun newInstance(listener: LogFilterListener): LogFilterDialogFragment {
            val fragment = LogFilterDialogFragment()
            fragment.listener = listener
            return fragment
        }
    }
    
    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        return MaterialAlertDialogBuilder(requireContext())
            .setTitle(R.string.filter_logs_title)
            .setMessage(R.string.filter_logs_description)
            .setPositiveButton(R.string.yes) { _, _ ->
                listener?.onFilterLogs(true)
            }
            .setNegativeButton(R.string.no) { _, _ ->
                listener?.onFilterLogs(false)
            }
            .create()
    }
}
