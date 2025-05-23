#include "effectmanagemenu.h"

#include "effects/effects_base/effectstypes.h"
#include "au3wrap/internal/wxtypes_convert.h"

using namespace muse;
using namespace muse::uicomponents;
using namespace muse::actions;
using namespace au::effects;

void EffectManageMenu::load()
{
    AbstractMenuModel::load();

    const EffectInstanceId instanceId = m_instanceId.toULongLong();
    const EffectId effectId = instancesRegister()->effectIdByInstanceId(instanceId);

    // subscribe on user presets change
    presetsController()->userPresetsChanged().onReceive(this, [this, effectId, instanceId](const EffectId& eid) {
        if (effectId != eid) {
            return;
        }

        reload(effectId, instanceId);
    });

    reload(effectId, instanceId);
}

void EffectManageMenu::reload(const EffectId& effectId, const EffectInstanceId& instanceId)
{
    MenuItemList items;
    QVariantList presets;

    presets << QVariantMap {
        { "id", "default" },
        { "name", muse::qtrc("effects", "Default preset") } };

    auto makeApplyAction = [](const EffectInstanceId& iid, const PresetId& p) {
        ActionQuery q("action://effects/presets/apply");
        q.addParam("instanceId", Val(iid));
        q.addParam("presetId", Val(au3::wxToStdSting(p)));
        return q;
    };

    // user
    PresetIdList userPresets = presetsController()->userPresets(effectId);
    {
        MenuItem* menuItem = makeMenu(TranslatableString("effects", "User Presets"), {});
        if (userPresets.empty()) {
            menuItem->setState(ui::UiActionState::make_disabled());
        } else {
            MenuItemList subitems;
            for (const PresetId& p : userPresets) {
                String name = au3::wxToString(p);
                MenuItem* item = makeMenuItem(makeApplyAction(instanceId, p).toString(), TranslatableString::untranslatable(name));
                item->setId("user_apply_" + name);
                subitems << item;

                presets << QVariantMap {
                    { "id", name.toQString() },
                    { "name", name.toQString() } };
            }
            menuItem->setSubitems(subitems);
        }
        items << menuItem;
    }

    {
        ActionQuery q("action://effects/presets/save");
        q.addParam("instanceId", Val(instanceId));
        MenuItem* item = makeMenuItem(q.toString());
        items << item;
    }

    {
        MenuItem* menuItem = makeMenu(TranslatableString("effects", "Delete Presets"), {});
        if (userPresets.empty()) {
            menuItem->setState(ui::UiActionState::make_disabled());
        } else {
            MenuItemList subitems;
            for (const PresetId& p : userPresets) {
                String name = au3::wxToString(p);
                ActionQuery q("action://effects/presets/delete");
                q.addParam("effectId", Val(effectId.toStdString()));
                q.addParam("presetId", Val(au3::wxToStdSting(p)));
                MenuItem* item = makeMenuItem(q.toString(), TranslatableString::untranslatable(name));
                item->setId("user_delete_" + name);
                subitems << item;
            }
            menuItem->setSubitems(subitems);
        }
        items << menuItem;
    }

    items << makeSeparator();

    // factory
    {
        PresetIdList factoryPresets = presetsController()->factoryPresets(effectId);
        MenuItem* menuItem = makeMenu(TranslatableString("effects", "Factory Presets"), {});

        MenuItemList subitems;
        MenuItem* defItem = makeMenuItem(makeApplyAction(instanceId, "default").toString(), TranslatableString("effects", "Defaults"));
        defItem->setId("factory_apply_default");
        subitems << defItem;

        for (const PresetId& p : factoryPresets) {
            String name = au3::wxToString(p);
            MenuItem* item = makeMenuItem(makeApplyAction(instanceId, p).toString(), TranslatableString::untranslatable(name));
            QString id = "factory_apply_" + name;
            item->setId(id);
            subitems << item;

            presets << QVariantMap {
                { "id", name.toQString() },
                { "name", name.toQString() } };
        }
        menuItem->setSubitems(subitems);

        items << menuItem;
    }

    items << makeSeparator();

    // import / export
    {
        ActionQuery q("action://effects/presets/import");
        q.addParam("instanceId", Val(instanceId));
        MenuItem* item = makeMenuItem(q.toString());
        items << item;
    }

    {
        ActionQuery q("action://effects/presets/export");
        q.addParam("instanceId", Val(instanceId));
        MenuItem* item = makeMenuItem(q.toString());
        items << item;
    }

    setItems(items);
    m_presets = presets;
    emit presetsChanged();
}

QString EffectManageMenu::instanceId_prop() const
{
    return m_instanceId;
}

void EffectManageMenu::setInstanceId_prop(const QString& newInstanceId)
{
    if (m_instanceId == newInstanceId) {
        return;
    }
    m_instanceId = newInstanceId;
    emit instanceIdChanged();
}

QVariantList EffectManageMenu::presets()
{
    return m_presets;
}

QString EffectManageMenu::preset() const
{
    return m_currentPreset;
}

void EffectManageMenu::setPreset(QString presetId)
{
    m_currentPreset = presetId;
    resetPreset();
    emit presetChanged();
}

void EffectManageMenu::resetPreset()
{
    const EffectInstanceId instanceId = m_instanceId.toULongLong();
    ActionQuery q("action://effects/presets/apply");
    q.addParam("instanceId", Val(instanceId));
    q.addParam("presetId", Val(m_currentPreset.toStdString()));
    dispatcher()->dispatch(q);
}

void EffectManageMenu::savePresetAs()
{
    const EffectInstanceId instanceId = m_instanceId.toULongLong();
    ActionQuery q("action://effects/presets/save");
    q.addParam("instanceId", Val(instanceId));
    dispatcher()->dispatch(q);
    emit presetsChanged();
    emit presetChanged();
}
