/* -*- mode: js2; js2-basic-offset: 2; indent-tabs-mode: nil -*- */

/* FIXME: ALT + F2 로 익스텐션을 여러번 반복하여 재실행하다보면
 * 응용 프로그램이 죽는 경우가 있습니다.
 * 익스텐션의 문제인지, WM, GNOME SHELL 문제인지, 아니면 im library의 문제인지 반드시 확인해봐야 합니다 */

const Clutter = imports.gi.Clutter;
const Lang = imports.lang;
const St = imports.gi.St;
const Dasom = imports.gi.Dasom;
const Tweener = imports.ui.tweener;

const Main = imports.ui.main;
const PanelMenu = imports.ui.panelMenu;
const PopupMenu = imports.ui.popupMenu;

const Gettext = imports.gettext.domain('gnome-shell-extensions');
const _ = Gettext.gettext;

let dasom_agent, dasom_menu, engine_label;

function _hideEngineLabel()
{
    Main.uiGroup.remove_actor(engine_label);
    engine_label = null;
}

function _showEngineLabel(text)
{
    if (!engine_label)
    {
        engine_label = new St.Label({ style_class: 'engine-label', text: text });
        Main.uiGroup.add_actor(engine_label);
    }

    engine_label.opacity = 255;
    engine_label.text = text;

    let [x, y, mods] = global.get_pointer();

    engine_label.set_position(x, y);

    Tweener.addTween(engine_label, { opacity: 0,
                                     time: 2,
                                     transition: 'easeOutQuad',
                                     onComplete: _hideEngineLabel });
}

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
        this.actor.add_actor(this._hbox);

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

    dasom_agent.connect('engine-changed', function(agent, text) {
        if (text != "Dasom")
        {
            dasom_menu._hbox.remove_child (dasom_menu._icon);

            if (dasom_menu._label.text != text)
            {
                dasom_menu._label.text = text;
                _showEngineLabel(text);
            }

            dasom_menu._hbox.add_child (dasom_menu._label);
        }
        else
        {
            dasom_menu._hbox.remove_child (dasom_menu._label);
            dasom_menu._hbox.add_child (dasom_menu._icon);
        }
    });

    Main.panel.addToStatusArea('dasom-agent-extension', dasom_menu, 1, 'right');
}

// FIXME: 사용 중에 disable() 함수가 호출되는 경우가 있습니다.
function disable()
{
    dasom_menu.destroy();
    // dasom_agent.destroy(); FIXME: 해제는 어떻게?
}
