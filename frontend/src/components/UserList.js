import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { ToastContainer, toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import Modal from 'react-modal';
import { Tooltip } from 'react-tooltip';

Modal.setAppElement('#root');

function UserList() {
  const [users, setUsers] = useState([]);
  const [modalIsOpen, setModalIsOpen] = useState(false);
  const [registerModalIsOpen, setRegisterModalIsOpen] = useState(false);
  const [currentUser, setCurrentUser] = useState(null);

  const [registerForm, setRegisterForm] = useState({
    email: "",
    password: "",
    role: "",
    created_date: new Date(),
    update_date: new Date(),
  });

  const roleOptions = [
    { value: "", label: "Select the role" },
    { value: "admin", label: "Admin" },
    { value: "user", label: "User" },
  ];

  const fetchUsers = async () => {
    try {
      const response = await axios.get('http://localhost:4000/api/users');
      if (Array.isArray(response.data.users)) {
        setUsers(response.data.users);
      } else {
        console.error('Expected an array but got:', typeof response.data.users);
        toast.error('Data format error: Expected an array.');
        setUsers([]);
      }
    } catch (error) {
      console.error('There was an error fetching users!', error);
      toast.error(error.response?.data?.detail || 'Failed to fetch users.');
    }
  };

  useEffect(() => {
    fetchUsers();
  }, []);

  const openModal = (user) => {
    setCurrentUser(user);
    setModalIsOpen(true);
  };

  const closeModal = () => {
    setModalIsOpen(false);
    setCurrentUser(null);
  };

  const handleEditUser = async (e) => {
    e.preventDefault();

    const password = e.target.elements.password.value;
    const role = e.target.elements.role.value;

    const user = {
      email: currentUser.email,
      password: password,
      role: role,
      created_date: currentUser.created_date,
      update_date: new Date(),
    };

    try {
      const response = await axios.put(`http://localhost:4000/api/users/${currentUser.email}`, user);
      toast.success(response.data.message || 'User updated successfully.');
      closeModal();
      await fetchUsers();
    } catch (error) {
      console.error('There was an error updating the user!', error);
      toast.error(error.response?.data?.detail || 'Failed to update user.');
    }
  };

  const deleteUser = async (email) => {
    if (email === "admin@admin.com") {
      toast.error("Cannot remove admin user");
      return;
    }

    try {
      await axios.delete(`http://localhost:4000/api/users/${email}`);
      toast.success('User deleted successfully.');
      await fetchUsers();
    } catch (error) {
      console.error('There was an error deleting the user!', error);
      toast.error(error.response?.data?.detail || 'Failed to delete user.');
    }
  };

  const onChangeForm = (label, event) => {
    setRegisterForm({ ...registerForm, [label]: event.target.value });
  };

  const onSubmitHandler = async (event) => {
    event.preventDefault();

    try {
      const response = await axios.post("http://localhost:4000/api/signup", registerForm);
      toast.success(response.data.detail || 'User registered successfully.');
      setRegisterModalIsOpen(false);
      await fetchUsers();
    } catch (error) {
      console.error('There was an error registering the user!', error);
      toast.error(error.response?.data?.detail || 'Failed to register user.');
    }
  };

  return (
    <React.Fragment>
      <div style={{ display: 'flex', justifyContent: 'center', marginTop: '10px'}}>
      <button
          type='button'
          className="block text-lg py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
          style={{ fontSize: '1.25rem' }}
          onClick={() => setRegisterModalIsOpen(true)}
        >
          Register New User
        </button>
      </div>
      <div style={{ display: 'flex', justifyContent: 'center' }}>
        <Modal isOpen={registerModalIsOpen} onRequestClose={() => setRegisterModalIsOpen(false)} style={{ content: { backgroundColor: '#0e2f48' } }}>
          <h2 style={{ color: 'white' }}>Register New User</h2>
          <form onSubmit={onSubmitHandler}>
            <input 
              type="email" 
              placeholder="Email" 
              className="block text-sm py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
              style={{ fontSize: '1.25rem' }}
              onChange={(event) => onChangeForm("email", event)}
              required 
            />
            <input 
              type="password" 
              placeholder="Password"
              className="block text-sm py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
              style={{ fontSize: '1.25rem' }}
              onChange={(event) => onChangeForm("password", event)}
              required 
              />
            <select 
              className="block text-sm py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
              style={{ fontSize: '1.25rem' }}
              onChange={(event) => onChangeForm("role", event)} 
              required
            >
              {roleOptions.map(option => (
                <option key={option.value} value={option.value}>{option.label}</option>
              ))}
            </select>
            <div className='row d-flex justify-content-center'>
              <div className='col-6 justify-content-center'>
                <button
                  id='submit-register-button'
                  type="submit"
                  className="rounded-pill"
                  style={{ fontSize: '1.25rem' }}
                ><i className="bi bi-check-lg"></i></button>
                <Tooltip anchorSelect="#submit-register-button" content="Register User" />
              </div>
              <div className='col-6 justify-content-center'>
                <button
                  id="close-register-button"
                  type="button"
                  onClick={() => setRegisterModalIsOpen(false)}
                  className="rounded-pill"
                  style={{ fontSize: '1.25rem' }}
                ><i className="bi bi-x-lg"></i></button>
                <Tooltip anchorSelect="#close-register-button" content="Close" />
              </div>
            </div>
          </form>
        </Modal>
        <table style={{ borderCollapse: 'separate', borderSpacing: '20px 20px', textAlign: 'center', fontSize: '1.25rem' }}>
          <thead>
            <tr>
              <th>Email</th>
              <th>Role</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {users.map(user => (
              <tr key={user.email}>
                <td>{user.email}</td>
                <td>{user.role}</td>
                <td><button
                      id="edit-tooltip"
                      type='button'
                      onClick={() => openModal(user)}
                      className="block text-lg py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
                      style={{ fontSize: '1.25rem' }}
                    ><i className="bi bi-pencil-square"></i></button>
                    <Tooltip anchorSelect="#edit-tooltip" content="Edit User" />
                    <button
                      id="delete-tooltip"
                      type='button'
                      onClick={() => deleteUser(user.email)}
                      className="block text-lg py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
                      style={{ fontSize: '1.25rem', color: 'white'}}
                    ><i className="bi bi-x-lg"></i></button>
                    <Tooltip anchorSelect="#delete-tooltip" content="Delete User" />
                </td>
              </tr>
            ))}
          </tbody>
        </table>
        <Modal isOpen={modalIsOpen} onRequestClose={closeModal} style={{ content: { backgroundColor: '#0e2f48' } }}>
          <h2 style={{ color: 'white' }}>Edit User</h2>
          <form onSubmit={handleEditUser}>
            <input 
              name="email" 
              type="email"
              className="block text-sm py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
              style={{ fontSize: '1.25rem' }}
              defaultValue={currentUser?.email} 
              disabled 
            />
            <input
              name="password" 
              type="password"
              className="block text-sm py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
              style={{ fontSize: '1.25rem' }}
              placeholder="New Password" 
              required
            />
            <select
              name='role'
              className="block text-sm py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
              style={{ fontSize: '1.25rem' }}
              onChange={(event) => onChangeForm("role", event)} 
              required
            >
              {roleOptions.map(option => (
                <option key={option.value} value={option.value}>{option.label}</option>
              ))}
            </select>
            <div className='container'>
              <div className='row d-flex justify-content-center'>
                <div className='col-6 d-flex justify-content-center'>
                  <button
                    id="submit-edit-button"
                    type="submit"
                    className="block text-lg py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
                    style={{ fontSize: '1.25rem' }}
                  ><i className="bi bi-check-lg"></i></button>
                  <Tooltip anchorSelect="#submit-edit-button" content="Edit User" />
                </div>
                <div className='col-6 d-flex justify-content-center'>
                  <button
                    id="close-edit-button"
                    type="button"
                    onClick={closeModal}
                    className="block text-lg py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
                    style={{ fontSize: '1.25rem' }}
                  ><i className="bi bi-x-lg"></i></button>
                  <Tooltip anchorSelect="#close-edit-button" content="Close" />
                </div>
              </div>
            </div>
          </form>
        </Modal>
        </div>
        <ToastContainer />
    </React.Fragment>
  );
}

export default UserList;