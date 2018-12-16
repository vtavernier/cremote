package com.vtavernier.cremote

import android.view.ContextMenu
import android.view.View

interface RecyclerViewClickListener {
    fun onClick(view : View?, position : Int)
    fun onCreateContextMenu(menu: ContextMenu, v: View, menuInfo: ContextMenu.ContextMenuInfo?, position: Int)
}