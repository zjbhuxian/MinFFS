// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0        *
// * Copyright (C) Zenju (zenju AT gmx DOT de) - All Rights Reserved        *
// **************************************************************************

#include "grid_view.h"
#include "sorting.h"
#include "../synchronization.h"
#include <zen/stl_tools.h>

using namespace zen;


template <class StatusResult>
void addNumbers(const FileSystemObject& fsObj, StatusResult& result)
{
    struct GetValues : public FSObjectVisitor
    {
        GetValues(StatusResult& res) : result_(res) {}

        void visit(const FilePair& file) override
        {
            if (!file.isEmpty<LEFT_SIDE>())
            {
                result_.filesizeLeftView += file.getFileSize<LEFT_SIDE>();
                ++result_.filesOnLeftView;
            }
            if (!file.isEmpty<RIGHT_SIDE>())
            {
                result_.filesizeRightView += file.getFileSize<RIGHT_SIDE>();
                ++result_.filesOnRightView;
            }
        }

        void visit(const SymlinkPair& symlink) override
        {
            if (!symlink.isEmpty<LEFT_SIDE>())
                ++result_.filesOnLeftView;

            if (!symlink.isEmpty<RIGHT_SIDE>())
                ++result_.filesOnRightView;
        }

        void visit(const FolderPair& folder) override
        {
            if (!folder.isEmpty<LEFT_SIDE>())
                ++result_.foldersOnLeftView;

            if (!folder.isEmpty<RIGHT_SIDE>())
                ++result_.foldersOnRightView;
        }
        StatusResult& result_;
    } getVal(result);
    fsObj.accept(getVal);
}


template <class Predicate>
void GridView::updateView(Predicate pred)
{
    viewRef.clear();
    rowPositions.clear();
    rowPositionsFirstChild.clear();

    std::for_each(sortedRef.begin(), sortedRef.end(),
                  [&](const RefIndex& ref)
    {
        if (const FileSystemObject* fsObj = FileSystemObject::retrieve(ref.objId))
            if (pred(*fsObj))
            {
                //save row position for direct random access to FilePair or FolderPair
                this->rowPositions.emplace(ref.objId, viewRef.size()); //costs: 0.28 �s per call - MSVC based on std::set
                //"this->" required by two-pass lookup as enforced by GCC 4.7

                //save row position to identify first child *on sorted subview* of FolderPair or BaseFolderPair in case latter are filtered out
                const HierarchyObject* parent = &fsObj->parent();
                for (;;) //map all yet unassociated parents to this row
                {
                    const auto rv = this->rowPositionsFirstChild.emplace(parent, viewRef.size());
                    if (!rv.second)
                        break;

                    if (auto folder = dynamic_cast<const FolderPair*>(parent))
                        parent = &(folder->parent());
                    else
                        break;
                }

                //build subview
                this->viewRef.push_back(ref.objId);
            }
    });
}


ptrdiff_t GridView::findRowDirect(FileSystemObject::ObjectIdConst objId) const
{
    auto it = rowPositions.find(objId);
    return it != rowPositions.end() ? it->second : -1;
}

ptrdiff_t GridView::findRowFirstChild(const HierarchyObject* hierObj) const
{
    auto it = rowPositionsFirstChild.find(hierObj);
    return it != rowPositionsFirstChild.end() ? it->second : -1;
}


GridView::StatusCmpResult GridView::updateCmpResult(bool showExcluded, //maps sortedRef to viewRef
                                                    bool leftOnlyFilesActive,
                                                    bool rightOnlyFilesActive,
                                                    bool leftNewerFilesActive,
                                                    bool rightNewerFilesActive,
                                                    bool differentFilesActive,
                                                    bool equalFilesActive,
                                                    bool conflictFilesActive)
{
    StatusCmpResult output;

    updateView([&](const FileSystemObject& fsObj) -> bool
    {
        if (!fsObj.isActive())
        {
            output.existsExcluded = true;
            if (!showExcluded)
                return false;
        }

        switch (fsObj.getCategory())
        {
            case FILE_LEFT_SIDE_ONLY:
                output.existsLeftOnly = true;
                if (!leftOnlyFilesActive) return false;
                break;
            case FILE_RIGHT_SIDE_ONLY:
                output.existsRightOnly = true;
                if (!rightOnlyFilesActive) return false;
                break;
            case FILE_LEFT_NEWER:
                output.existsLeftNewer = true;
                if (!leftNewerFilesActive) return false;
                break;
            case FILE_RIGHT_NEWER:
                output.existsRightNewer = true;
                if (!rightNewerFilesActive) return false;
                break;
            case FILE_DIFFERENT_CONTENT:
                output.existsDifferent = true;
                if (!differentFilesActive) return false;
                break;
            case FILE_EQUAL:
            case FILE_DIFFERENT_METADATA: //= sub-category of equal
                output.existsEqual = true;
                if (!equalFilesActive) return false;
                break;
            case FILE_CONFLICT:
                output.existsConflict = true;
                if (!conflictFilesActive) return false;
                break;
        }
        //calculate total number of bytes for each side
        addNumbers(fsObj, output);
        return true;
    });

    return output;
}


GridView::StatusSyncPreview GridView::updateSyncPreview(bool showExcluded, //maps sortedRef to viewRef
                                                        bool syncCreateLeftActive,
                                                        bool syncCreateRightActive,
                                                        bool syncDeleteLeftActive,
                                                        bool syncDeleteRightActive,
                                                        bool syncDirOverwLeftActive,
                                                        bool syncDirOverwRightActive,
                                                        bool syncDirNoneActive,
                                                        bool syncEqualActive,
                                                        bool conflictFilesActive)
{
    StatusSyncPreview output;

    updateView([&](const FileSystemObject& fsObj) -> bool
    {
        if (!fsObj.isActive())
        {
            output.existsExcluded = true;
            if (!showExcluded)
                return false;
        }

        switch (fsObj.getSyncOperation()) //evaluate comparison result and sync direction
        {
            case SO_CREATE_NEW_LEFT:
                output.existsSyncCreateLeft = true;
                if (!syncCreateLeftActive) return false;
                break;
            case SO_CREATE_NEW_RIGHT:
                output.existsSyncCreateRight = true;
                if (!syncCreateRightActive) return false;
                break;
            case SO_DELETE_LEFT:
                output.existsSyncDeleteLeft = true;
                if (!syncDeleteLeftActive) return false;
                break;
            case SO_DELETE_RIGHT:
                output.existsSyncDeleteRight = true;
                if (!syncDeleteRightActive) return false;
                break;
            case SO_OVERWRITE_RIGHT:
            case SO_COPY_METADATA_TO_RIGHT: //no extra button on screen
            case SO_MOVE_RIGHT_SOURCE:
            case SO_MOVE_RIGHT_TARGET:
                output.existsSyncDirRight = true;
                if (!syncDirOverwRightActive) return false;
                break;
            case SO_OVERWRITE_LEFT:
            case SO_COPY_METADATA_TO_LEFT: //no extra button on screen
            case SO_MOVE_LEFT_TARGET:
            case SO_MOVE_LEFT_SOURCE:
                output.existsSyncDirLeft = true;
                if (!syncDirOverwLeftActive) return false;
                break;
            case SO_DO_NOTHING:
                output.existsSyncDirNone = true;
                if (!syncDirNoneActive) return false;
                break;
            case SO_EQUAL:
                output.existsEqual = true;
                if (!syncEqualActive) return false;
                break;
            case SO_UNRESOLVED_CONFLICT:
                output.existsConflict = true;
                if (!conflictFilesActive) return false;
                break;
        }

        //calculate total number of bytes for each side
        addNumbers(fsObj, output);
        return true;
    });

    return output;
}


std::vector<FileSystemObject*> GridView::getAllFileRef(const std::vector<size_t>& rows)
{
    const size_t viewSize = viewRef.size();

    std::vector<FileSystemObject*> output;

    for (size_t pos : rows)
        if (pos < viewSize)
            if (FileSystemObject* fsObj = FileSystemObject::retrieve(viewRef[pos]))
                output.push_back(fsObj);

    return output;
}


void GridView::removeInvalidRows()
{
    viewRef.clear();
    rowPositions.clear();
    rowPositionsFirstChild.clear();

    //remove rows that have been deleted meanwhile
    erase_if(sortedRef, [&](const RefIndex& refIdx) { return FileSystemObject::retrieve(refIdx.objId) == nullptr; });
}


class GridView::SerializeHierarchy
{
public:
    static void execute(HierarchyObject& hierObj, std::vector<GridView::RefIndex>& sortedRef, size_t index) { SerializeHierarchy(sortedRef, index).recurse(hierObj); }

private:
    SerializeHierarchy(std::vector<GridView::RefIndex>& sortedRef, size_t index) :
        index_(index),
        sortedRef_(sortedRef) {}

    void recurse(HierarchyObject& hierObj)
    {
        for (FilePair& file : hierObj.refSubFiles())
            sortedRef_.emplace_back(index_, file.getId());
        for (SymlinkPair& symlink : hierObj.refSubLinks())
            sortedRef_.emplace_back(index_, symlink.getId());
        for (FolderPair& folder : hierObj.refSubFolders())
        {
            sortedRef_.emplace_back(index_, folder.getId());
            recurse(folder); //add recursion here to list sub-objects directly below parent!
        }
    }

    size_t index_;
    std::vector<GridView::RefIndex>& sortedRef_;
};


void GridView::setData(FolderComparison& folderCmp)
{
    //clear everything
    std::vector<FileSystemObject::ObjectId>().swap(viewRef); //free mem
    std::vector<RefIndex>().swap(sortedRef);                 //
    currentSort = NoValue();

    folderPairCount = std::count_if(begin(folderCmp), end(folderCmp),
                                    [](const BaseFolderPair& baseObj) //count non-empty pairs to distinguish single/multiple folder pair cases
    {
        return !AFS::isNullPath(baseObj.getAbstractPath<LEFT_SIDE >()) ||
               !AFS::isNullPath(baseObj.getAbstractPath<RIGHT_SIDE>());
    });

    for (auto it = begin(folderCmp); it != end(folderCmp); ++it)
        SerializeHierarchy::execute(*it, sortedRef, it - begin(folderCmp));
}


//------------------------------------ SORTING TEMPLATES ------------------------------------------------
template <bool ascending, SelectedSide side>
struct GridView::LessFullPath
{
    bool operator()(const RefIndex a, const RefIndex b) const
    {
        const FileSystemObject* fsObjA = FileSystemObject::retrieve(a.objId);
        const FileSystemObject* fsObjB = FileSystemObject::retrieve(b.objId);
        if (!fsObjA) //invalid rows shall appear at the end
            return false;
        else if (!fsObjB)
            return true;

        return lessFullPath<ascending, side>(*fsObjA, *fsObjB);
    }
};


template <bool ascending>
struct GridView::LessRelativeFolder
{
    bool operator()(const RefIndex a, const RefIndex b) const
    {
        const FileSystemObject* fsObjA = FileSystemObject::retrieve(a.objId);
        const FileSystemObject* fsObjB = FileSystemObject::retrieve(b.objId);
        if (!fsObjA) //invalid rows shall appear at the end
            return false;
        else if (!fsObjB)
            return true;

        //presort by folder pair
        if (a.folderIndex != b.folderIndex)
            return ascending ?
                   a.folderIndex < b.folderIndex :
                   a.folderIndex > b.folderIndex;

        return lessRelativeFolder<ascending>(*fsObjA, *fsObjB);
    }
};


template <bool ascending, SelectedSide side>
struct GridView::LessShortFileName
{
    bool operator()(const RefIndex a, const RefIndex b) const
    {
        const FileSystemObject* fsObjA = FileSystemObject::retrieve(a.objId);
        const FileSystemObject* fsObjB = FileSystemObject::retrieve(b.objId);
        if (!fsObjA) //invalid rows shall appear at the end
            return false;
        else if (!fsObjB)
            return true;

        return lessShortFileName<ascending, side>(*fsObjA, *fsObjB);
    }
};


template <bool ascending, SelectedSide side>
struct GridView::LessFilesize
{
    bool operator()(const RefIndex a, const RefIndex b) const
    {
        const FileSystemObject* fsObjA = FileSystemObject::retrieve(a.objId);
        const FileSystemObject* fsObjB = FileSystemObject::retrieve(b.objId);
        if (!fsObjA) //invalid rows shall appear at the end
            return false;
        else if (!fsObjB)
            return true;

        return lessFilesize<ascending, side>(*fsObjA, *fsObjB);
    }
};


template <bool ascending, SelectedSide side>
struct GridView::LessFiletime
{
    bool operator()(const RefIndex a, const RefIndex b) const
    {
        const FileSystemObject* fsObjA = FileSystemObject::retrieve(a.objId);
        const FileSystemObject* fsObjB = FileSystemObject::retrieve(b.objId);
        if (!fsObjA) //invalid rows shall appear at the end
            return false;
        else if (!fsObjB)
            return true;

        return lessFiletime<ascending, side>(*fsObjA, *fsObjB);
    }
};


template <bool ascending, SelectedSide side>
struct GridView::LessExtension
{
    bool operator()(const RefIndex a, const RefIndex b) const
    {
        const FileSystemObject* fsObjA = FileSystemObject::retrieve(a.objId);
        const FileSystemObject* fsObjB = FileSystemObject::retrieve(b.objId);
        if (!fsObjA) //invalid rows shall appear at the end
            return false;
        else if (!fsObjB)
            return true;

        return lessExtension<ascending, side>(*fsObjA, *fsObjB);
    }
};


template <bool ascending>
struct GridView::LessCmpResult
{
    bool operator()(const RefIndex a, const RefIndex b) const
    {
        const FileSystemObject* fsObjA = FileSystemObject::retrieve(a.objId);
        const FileSystemObject* fsObjB = FileSystemObject::retrieve(b.objId);
        if (!fsObjA) //invalid rows shall appear at the end
            return false;
        else if (!fsObjB)
            return true;

        return lessCmpResult<ascending>(*fsObjA, *fsObjB);
    }
};


template <bool ascending>
struct GridView::LessSyncDirection
{
    bool operator()(const RefIndex a, const RefIndex b) const
    {
        const FileSystemObject* fsObjA = FileSystemObject::retrieve(a.objId);
        const FileSystemObject* fsObjB = FileSystemObject::retrieve(b.objId);
        if (!fsObjA) //invalid rows shall appear at the end
            return false;
        else if (!fsObjB)
            return true;

        return lessSyncDirection<ascending>(*fsObjA, *fsObjB);
    }
};

//-------------------------------------------------------------------------------------------------------
bool GridView::getDefaultSortDirection(ColumnTypeRim type) //true: ascending; false: descending
{
    switch (type)
    {
        case COL_TYPE_SIZE:
        case COL_TYPE_DATE:
            return false;

        case COL_TYPE_BASE_DIRECTORY:
        case COL_TYPE_FULL_PATH:
        case COL_TYPE_REL_FOLDER:
        case COL_TYPE_FILENAME:
        case COL_TYPE_EXTENSION:
            return true;
    }
    assert(false);
    return true;
}


void GridView::sortView(ColumnTypeRim type, bool onLeft, bool ascending)
{
    viewRef.clear();
    rowPositions.clear();
    rowPositionsFirstChild.clear();
    currentSort = SortInfo(type, onLeft, ascending);

    switch (type)
    {
        case COL_TYPE_FULL_PATH:
            if      ( ascending &&  onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFullPath<true,  LEFT_SIDE >());
            else if ( ascending && !onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFullPath<true,  RIGHT_SIDE>());
            else if (!ascending &&  onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFullPath<false, LEFT_SIDE >());
            else if (!ascending && !onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFullPath<false, RIGHT_SIDE>());
            break;
        case COL_TYPE_REL_FOLDER:
            if      ( ascending) std::sort(sortedRef.begin(), sortedRef.end(), LessRelativeFolder<true>());
            else if (!ascending) std::sort(sortedRef.begin(), sortedRef.end(), LessRelativeFolder<false>());
            break;
        case COL_TYPE_FILENAME:
            if      ( ascending &&  onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessShortFileName<true,  LEFT_SIDE >());
            else if ( ascending && !onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessShortFileName<true,  RIGHT_SIDE>());
            else if (!ascending &&  onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessShortFileName<false, LEFT_SIDE >());
            else if (!ascending && !onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessShortFileName<false, RIGHT_SIDE>());
            break;
        case COL_TYPE_SIZE:
            if      ( ascending &&  onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFilesize<true,  LEFT_SIDE >());
            else if ( ascending && !onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFilesize<true,  RIGHT_SIDE>());
            else if (!ascending &&  onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFilesize<false, LEFT_SIDE >());
            else if (!ascending && !onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFilesize<false, RIGHT_SIDE>());
            break;
        case COL_TYPE_DATE:
            if      ( ascending &&  onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFiletime<true,  LEFT_SIDE >());
            else if ( ascending && !onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFiletime<true,  RIGHT_SIDE>());
            else if (!ascending &&  onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFiletime<false, LEFT_SIDE >());
            else if (!ascending && !onLeft) std::sort(sortedRef.begin(), sortedRef.end(), LessFiletime<false, RIGHT_SIDE>());
            break;
        case COL_TYPE_EXTENSION:
            if      ( ascending &&  onLeft) std::stable_sort(sortedRef.begin(), sortedRef.end(), LessExtension<true,  LEFT_SIDE >());
            else if ( ascending && !onLeft) std::stable_sort(sortedRef.begin(), sortedRef.end(), LessExtension<true,  RIGHT_SIDE>());
            else if (!ascending &&  onLeft) std::stable_sort(sortedRef.begin(), sortedRef.end(), LessExtension<false, LEFT_SIDE >());
            else if (!ascending && !onLeft) std::stable_sort(sortedRef.begin(), sortedRef.end(), LessExtension<false, RIGHT_SIDE>());
            break;
        //case SORT_BY_CMP_RESULT:
        //    if      ( ascending) std::stable_sort(sortedRef.begin(), sortedRef.end(), LessCmpResult<true >());
        //    else if (!ascending) std::stable_sort(sortedRef.begin(), sortedRef.end(), LessCmpResult<false>());
        //    break;
        //case SORT_BY_SYNC_DIRECTION:
        //    if      ( ascending) std::stable_sort(sortedRef.begin(), sortedRef.end(), LessSyncDirection<true >());
        //    else if (!ascending) std::stable_sort(sortedRef.begin(), sortedRef.end(), LessSyncDirection<false>());
        //    break;
        case COL_TYPE_BASE_DIRECTORY:
            if      ( ascending) std::stable_sort(sortedRef.begin(), sortedRef.end(), [](const RefIndex a, const RefIndex b) { return a.folderIndex < b.folderIndex; });
            else if (!ascending) std::stable_sort(sortedRef.begin(), sortedRef.end(), [](const RefIndex a, const RefIndex b) { return a.folderIndex > b.folderIndex; });
            break;
    }
}
