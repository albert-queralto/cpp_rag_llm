import React, { useState, useEffect } from "react";
import "bootstrap/dist/css/bootstrap.min.css";
import "bootstrap-icons/font/bootstrap-icons.css";

import Login from "./components/Login";
// import Register from "./components/Register";
import Forgot from "./components/Forgot";
import FileForm from "./components/FileForm";
import ChatInterface from "./components/ChatInterface";
// import RemoveUser from "./components/RemoveUser";
import UserList from "./components/UserList";
import CollectionsManagement from "./components/CollectionsManagement";
import "./App.css";

function App() {
    const [page, setPage] = useState("login");
    const [token, setToken] = useState();
    const [userRole, setUserRole] = useState();
    const [menuSelection, setMenuSelection] = useState();
    const [isMenuOpen, setIsMenuOpen] = useState(false);
    const toggleMenu = () => setIsMenuOpen(!isMenuOpen);

    useEffect(() => {
        const auth = localStorage.getItem("auth_token");
        const role = localStorage.getItem("user_role");
        setToken(auth);
        setUserRole(role);
    }, []);

    const chosePage = () => {
        switch (page) {
            case "login":
                return <Login setPage={setPage} />;
            // case "fileform":
            //     if (token) {
            //         return <FileForm />;
            //     } else {
            //         return <Login setPage={setPage} />;
            //     }
            // case "usermanagement":
            //     if (token && userRole === "admin") {
            //         return <UserList />;
            //     } else {
            //         return <Login setPage={setPage} />;
            //     }
            // case "collections":
            //     if (token && userRole === "admin") {
            //         return <Collections />;
            //     } else {
            //         return <Login setPage={setPage} />;
            //     }
            case "forgot":
                return <Forgot setPage={setPage} />;
            default:
                return <Login setPage={setPage} />;
        }
    };

    const renderMenuSelection = () => {
        switch (menuSelection) {
            case "chat":
                return <ChatInterface />;
            case "fileform":
                return <FileForm />;
            case "usermanagement":
                return <UserList />;
            case "collections":
                return <CollectionsManagement />;
            case "logout":
                localStorage.removeItem("auth_token");
                localStorage.removeItem("user_role");
                window.location.reload();
                break;
            default:
                return <ChatInterface />;
        }
    };

    const handleMenuSelection = (selection) => {
        setMenuSelection(selection);
        setIsMenuOpen(false);
    };

    const renderMenu = () => (
        <header className="chat-header">
            <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", width: "100%" }}>
                <div style={{ display: "flex", alignItems: "center" }}>
                    <div onClick={toggleMenu} style={{ cursor: 'pointer' }}>
                        <div style={{ width: '30px', height: '3px', backgroundColor: 'white', marginBottom: '5px' }}></div>
                        <div style={{ width: '30px', height: '3px', backgroundColor: 'white', marginBottom: '5px' }}></div>
                        <div style={{ width: '30px', height: '3px', backgroundColor: 'white' }}></div>
                    </div>
                    {isMenuOpen && (
                        <div className="hamburger-menu">
                            <button className="navbar-btn" onClick={() => handleMenuSelection("chat")}>Chat</button>
                            {userRole === "admin" && (
                                <>
                                    <button className="navbar-btn" onClick={() => handleMenuSelection("fileform")}>File Management</button>
                                    <button className="navbar-btn" onClick={() => handleMenuSelection("usermanagement")}>User Management</button>
                                    <button className="navbar-btn" onClick={() => handleMenuSelection("collections")}>Collections Management</button>
                                    {/* <button className="navbar-btn" onClick={() => handleMenuSelection("register")}>Register</button>
                                    <button className="navbar-btn" onClick={() => handleMenuSelection("remove")}>Remove User</button> */}
                                </>
                            )}
                            <button className="navbar-btn" onClick={() => handleMenuSelection("logout")}>Logout</button>
                        </div>
                    )}
                </div>
            </div>
        </header>
    );

    const pages = () => {
        if (token == null) {
            return (
                <div className="min-h-screen bg-yellow-400 flex justify-center items-center">
                    <div className="py-12 px-12 bg-white rounded-2xl shadow-xl z-20">
                        {chosePage()}
                    </div>
                </div>
            );
        } else {
            return (
                <div>
                    {renderMenu()}
                    {renderMenuSelection()}
                </div>
            );
        }
    };

    return <React.Fragment>{pages()}</React.Fragment>;
}

export default App;