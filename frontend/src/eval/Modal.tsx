// based on code from https://medium.com/@pppped/build-a-simple-modal-window-with-react-and-jss-f05041d899cc
import * as React from "react";

import "./Modal.css";

interface ModalProps {
  // @ts-ignore
  onCloseRequest: () => void;
}
interface ModalState {}

export default class Modal extends React.Component<ModalProps, ModalState> {
  private modal: HTMLDivElement | null;

  constructor(props: ModalProps) {
    super(props);

    this.handleKeyUp = this.handleKeyUp.bind(this);
    this.handleOutsideClick = this.handleOutsideClick.bind(this);
  }

  public componentDidMount() {
    window.addEventListener("keyup", this.handleKeyUp, false);
    document.addEventListener("click", this.handleOutsideClick, false);
  }

  public componentWillUnmount() {
    window.removeEventListener("keyup", this.handleKeyUp, false);
    document.removeEventListener("click", this.handleOutsideClick, false);
  }

  public render() {
    const { onCloseRequest, children } = this.props;

    return (
      <div className="modalOverlay">
        <div className="modalCustom" ref={node => (this.modal = node)}>
          <div className="modalContent">{children}</div>
        </div>
        <button type="button" className="closeButton" onClick={() => onCloseRequest()} />
      </div>
    );
  }

  private handleKeyUp(e: KeyboardEvent) {
    const { onCloseRequest } = this.props;
    if (e.keyCode === 27) {
      e.preventDefault();
      onCloseRequest();
      window.removeEventListener("keyup", this.handleKeyUp, false);
    }
  }

  private handleOutsideClick(e: KeyboardEvent) {
    const { onCloseRequest } = this.props;
    if (this.modal) {
      // @ts-ignore
      if (!this.modal.contains(e.target)) {
        onCloseRequest();
        document.removeEventListener("click", this.handleOutsideClick, false);
      }
    }
  }
}
