/* -*- mode: js2; js2-basic-offset: 2; indent-tabs-mode: nil -*- */

const Clutter = imports.gi.Clutter;
const Lang = imports.lang;
const St = imports.gi.St;
const Dasom = imports.gi.Dasom;

const Main = imports.ui.main;
const PanelMenu = imports.ui.panelMenu;
const PopupMenu = imports.ui.popupMenu;

const Gettext = imports.gettext.domain('gnome-shell-extensions');
const _ = Gettext.gettext;

let dasom_agent, dasom_menu;

const DasomMenu = new Lang.Class({
    Name: 'DasomMenu',
    Extends: PanelMenu.Button,

    _init: function() {
        this.parent(0.0, _("Dasom"));

        this._hbox = new St.BoxLayout({ style_class: 'panel-status-menu-box' });
        this._label = new St.Label({ text: _("Dasom"),
                                     y_expand: true,
                                     y_align: Clutter.ActorAlign.CENTER });

        this._icon = new St.Icon({ icon_name: 'input-keyboard-symbolic',
                                   style_class: 'system-status-icon' });

        this._hbox.add_child(this._icon);
        this.actor.add_child(this._hbox);

        this.menu.addMenuItem(new PopupMenu.PopupSeparatorMenuItem());
        this._about_menu = new PopupMenu.PopupMenuItem(_("About"));

        this.menu.addMenuItem(this._about_menu);
    },
});

function init()
{
}

function enable()
{
    dasom_agent = new Dasom.Agent;
    dasom_menu  = new DasomMenu;
    let child;

    dasom_agent.connect('engine-changed', function(agent, text) {
        child = dasom_menu._hbox.get_child_at_index (0);

        if (text != "Dasom")
        {
            if (dasom_menu._label.text != text)
                dasom_menu._label.text = text;

            if (child != dasom_menu._label)
                dasom_menu._hbox.replace_child (child, dasom_menu._label);
        }
        else
        {
            if (child != dasom_menu._icon)
                dasom_menu._hbox.replace_child (child, dasom_menu._icon);
        }
    });

    Main.panel.addToStatusArea('dasom-agent-extension', dasom_menu, 0, 'right');
}

// FIXME: 사용 중에 disable() 함수가 호출되는 경우가 있습니다.
function disable()
{
    dasom_menu.destroy();
    // dasom_agent.destroy(); FIXME: How to free ?
}
