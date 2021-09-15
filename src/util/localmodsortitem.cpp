#include "localmodsortitem.h"

#include "local/localmod.h"

LocalModSortItem::LocalModSortItem(LocalMod *mod) :
    mod_(mod)
{

}

bool LocalModSortItem::operator<(const QListWidgetItem &other) const
{
    auto otherMod = dynamic_cast<const LocalModSortItem&>(other).mod_;
    switch (sortRule_) {
    case Name:
        return mod_->modInfo().name().toLower() < otherMod->modInfo().name().toLower();
    case Date:
        return mod_->modFile()->fileInfo().birthTime() < otherMod->modFile()->fileInfo().birthTime();
    case Size:
        // >
        return mod_->modFile()->fileInfo().size() > otherMod->modFile()->fileInfo().size();
    default:
        return true;
    }
}

LocalMod *LocalModSortItem::mod() const
{
    return mod_;
}

void LocalModSortItem::setSortRule(SortRule newSortRule)
{
    sortRule_ = newSortRule;
}
