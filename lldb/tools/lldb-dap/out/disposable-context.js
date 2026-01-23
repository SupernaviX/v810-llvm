"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.DisposableContext = void 0;
/**
 * This class provides a simple wrapper around vscode.Disposable that allows
 * for registering additional disposables.
 */
class DisposableContext {
    constructor() {
        this._disposables = [];
    }
    dispose() {
        for (const disposable of this._disposables) {
            disposable.dispose();
        }
        this._disposables = [];
    }
    /**
     * Push an additional disposable to the context.
     *
     * @param disposable The disposable to register.
     */
    pushSubscription(disposable) {
        this._disposables.push(disposable);
    }
}
exports.DisposableContext = DisposableContext;
//# sourceMappingURL=disposable-context.js.map