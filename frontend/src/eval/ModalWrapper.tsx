// based on code from https://medium.com/@pppped/build-a-simple-modal-window-with-react-and-jss-f05041d899cc
import * as React from "react";
import Modal from "./Modal";

import "./ModalWrapper.css";

interface ModalWrapperProps {
  buttonLabel: string;
}
interface ModalWrapperState {
  showModal: boolean;
}

export default class ModalWrapper extends React.Component<ModalWrapperProps, ModalWrapperState> {
  constructor(props: ModalWrapperProps) {
    super(props);
    this.state = {
      showModal: false
    };
  }
  public render() {
    const { buttonLabel, children } = this.props;
    const { showModal } = this.state;
    return (
      <div className="modal-wrapper">
        <button
          type="button"
          className="btn btn-outline-primary"
          onClick={() => this.handleToggleModal()}
        >
          {buttonLabel}
        </button>
        {showModal && <Modal onCloseRequest={() => this.handleToggleModal()}>{children}</Modal>}
      </div>
    );
  }

  private handleToggleModal() {
    this.setState({ showModal: !this.state.showModal });
  }
}
