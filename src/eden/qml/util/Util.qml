pragma Singleton

import QtQuick

QtObject {
    /**
     * Recursively search an Item for children matching the specified type.
     * @return A list of found items.
     */
    function searchItem(parent, typeName) {
        let results = []

        // Search contentChildren too for views/layouts/etc
        let children = parent.children
        if (parent.contentChildren) {
            children = parent.contentChildren
        }

        for (var i = 0; i < children.length; ++i) {
            let child = children[i]

            if (child.typeName === typeName) {
                results.push(child)
            }

            let childResults = searchItem(child, typeName)
            results = results.concat(childResults)
        }

        return results
    }
}
